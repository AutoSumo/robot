#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <esp_wifi.h>
#include "connection-details.h"
#include "packets.h"


#define DRIVER_STANDBY 19

#define DRIVER_PWM_A 22
#define DRIVER_PWM_CHANNEL_A 0
#define DRIVER_IN_1_A 21
#define DRIVER_IN_2_A 23

#define DRIVER_PWM_B 17
#define DRIVER_PWM_CHANNEL_B 1
#define DRIVER_IN_1_B 18
#define DRIVER_IN_2_B 5

#define SERVO_PIN 16

#define LEFT_TACH_A 35
#define LEFT_TACH_D 34
#define RIGHT_TACH_A 39
#define RIGHT_TACH_D 36

#define IR_LEFT 33
#define IR_RIGHT 32

#define LIDAR_SDA 27
#define LIDAR_SCL 26
#define LIDAR_INT 25


#define SPEED_TIMEOUT_MS 1000
#define LIDAR_INTERVAL 100

int leftSpeed = 0;
int rightSpeed = 0;
unsigned long lastSpeedRequest = 0;
bool speedUpdated = false;

uint8_t servoPos = 90;
bool servoUpdated = false;

bool lastIRLeft = false;
bool lastIRRight = false;

unsigned long lidarLastSent = 0;

WebSocketsClient webSocket;
Servo servo;

Adafruit_VL6180X vl = Adafruit_VL6180X();
TwoWire lidarTwoWire = TwoWire(0);

void onWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.println("Disconnected from websocket!");
			break;
		case WStype_CONNECTED:
			Serial.println("Connected to websocket!");
            webSocket.sendTXT(ROBOT_ID);
			break;
		case WStype_TEXT:
            // Send text data back
            webSocket.sendTXT((char*) payload);
			break;
		case WStype_BIN:
            if(length < 1) break;

            PacketType packetType;
            memcpy(&packetType, payload, sizeof(PacketType));

            if(packetType == MOVE_MOTORS) {
                if(length < (sizeof(MoveMotorsPacket) + 1)) break;

                MoveMotorsPacket packet;
                memcpy(&packet, payload+1, sizeof(MoveMotorsPacket));
                leftSpeed = packet.leftPower * (packet.leftDirection == 1 ? 1 : -1);
                rightSpeed = packet.rightPower * (packet.rightDirection == 1 ? 1 : -1);
                lastSpeedRequest = millis();
                speedUpdated = true;
            } else if (packetType == MOVE_SERVO) {
                if(length < (sizeof(MoveServoPacket) + 1)) break;

                MoveServoPacket packet;
                memcpy(&packet, payload+1, sizeof(MoveServoPacket));
                servoPos = packet.angle;
                servoUpdated = true;
            }
            
			break;
		case WStype_ERROR:			
            Serial.println("Websocket error!");
            break;
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
		case WStype_PING:
		case WStype_PONG:
			break;
	}

}

void websocketTask(void *pvParameters) {
    // Connect to websocket server
    Serial.print("WS running on core ");
    Serial.println(xPortGetCoreID());

    webSocket.onEvent(onWebSocketEvent);
    webSocket.begin(WEBSOCKET_IP, WEBSOCKET_PORT, "/");
    while(true) {
        webSocket.loop();
    }
}

TaskHandle_t wsTask;

void setup() {
    Serial.begin(115200);
    Serial.println("");

    // Setup pins
    // Motor driver
    pinMode(DRIVER_IN_1_A, OUTPUT);
    pinMode(DRIVER_IN_2_A, OUTPUT);
    pinMode(DRIVER_IN_1_B, OUTPUT);
    pinMode(DRIVER_IN_2_B, OUTPUT);

    pinMode(DRIVER_STANDBY, OUTPUT);

    ledcAttachPin(DRIVER_PWM_A, DRIVER_PWM_CHANNEL_A);
    ledcSetup(DRIVER_PWM_CHANNEL_A, 5000, 8);

    ledcAttachPin(DRIVER_PWM_B, DRIVER_PWM_CHANNEL_B);
    ledcSetup(DRIVER_PWM_CHANNEL_B, 5000, 8);

    digitalWrite(DRIVER_STANDBY, HIGH);

    // IR sensors
    pinMode(IR_LEFT, INPUT);
    pinMode(IR_RIGHT, INPUT);

    // Servo
    servo.attach(SERVO_PIN);

    // Lidar
    lidarTwoWire.begin(LIDAR_SDA, LIDAR_SCL, 100000);
    vl.begin(&lidarTwoWire);

    // Connect to wifi
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    xTaskCreatePinnedToCore(websocketTask, "WS Task", 10000, NULL, 0, &wsTask, 0);
}

void sendIR(bool left, bool right) {
    IRDataPacket irDataPacket;
    irDataPacket.type = IR_DATA;
    irDataPacket.left = left;
    irDataPacket.right = right;

    webSocket.sendBIN((uint8_t*) &irDataPacket, sizeof(IRDataPacket));
}

void sendLidar(uint8_t range, uint8_t status) {
    LidarDataPacket lidarDataPacket;
    lidarDataPacket.type = LIDAR_DATA;
    lidarDataPacket.range = range;
    lidarDataPacket.status = status;
    webSocket.sendBIN((uint8_t*) &lidarDataPacket, sizeof(LidarDataPacket));
}

void loop() {
    delay(1);

    unsigned long now = millis();

    if(now - lastSpeedRequest >= SPEED_TIMEOUT_MS) {
        leftSpeed = 0;
        rightSpeed = 0;
        speedUpdated = true;
    }

    if(speedUpdated) {
        if(leftSpeed > 0) {
            digitalWrite(DRIVER_IN_1_A, LOW);
            digitalWrite(DRIVER_IN_2_A, HIGH);
        } else {
            digitalWrite(DRIVER_IN_1_A, HIGH);
            digitalWrite(DRIVER_IN_2_A, LOW);
        }
        ledcWrite(DRIVER_PWM_CHANNEL_A, abs(leftSpeed));

        if(rightSpeed > 0) {
            digitalWrite(DRIVER_IN_1_B, LOW);
            digitalWrite(DRIVER_IN_2_B, HIGH);
        } else {
            digitalWrite(DRIVER_IN_1_B, HIGH);
            digitalWrite(DRIVER_IN_2_B, LOW);
        }
        ledcWrite(DRIVER_PWM_CHANNEL_B, abs(rightSpeed));

        speedUpdated = false;
    }

    if(servoUpdated) {
        servo.write(servoPos);
        servoUpdated = false;
    }

    if(webSocket.isConnected()) {
        // Read and send IR data
        bool irLeft = digitalRead(IR_LEFT) == HIGH ? true : false;
        bool irRight = digitalRead(IR_RIGHT) == HIGH ? true : false;
        if(irLeft != lastIRLeft || irRight != lastIRRight) {
            lastIRLeft = irLeft;
            lastIRRight = irRight;
            sendIR(irLeft, irRight);
        }

        // Send LiDAR data
        if(now - lidarLastSent > LIDAR_INTERVAL) {
            uint8_t range = vl.readRange();
            uint8_t status = vl.readRangeStatus();
            sendLidar(range, status);
            lidarLastSent = now;
        }
    }
}