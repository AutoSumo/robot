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


int leftSpeed = 0;
int rightSpeed = 0;
unsigned long lastSpeedRequest = 0;
bool speedUpdated = false;

WebSocketsClient webSocket;

void onWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.println("Disconnected from websocket!");
			break;
		case WStype_CONNECTED:
			Serial.println("Connected to websocket!");
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
                if(length < sizeof(MoveMotorsPacket)) break;

                MoveMotorsPacket packet;
                memcpy(&packet, payload+1, sizeof(MoveMotorsPacket));
                Serial.print("Left power: ");
                Serial.print(packet.leftPower * (packet.leftDirection == 1 ? 1 : -1));
                Serial.print(" Right power: ");
                Serial.println(packet.rightPower * (packet.rightDirection == 1 ? 1 : -1));
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

void loop() {
    //webSocket.loop();
    delay(1);
}