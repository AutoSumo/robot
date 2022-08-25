# AutoSumo Robot

Robot used in the AutoSumo arena. Remote controlled by the [Bot Server](https://github.com/AutoSumo/server) using websockets.

```mermaid
flowchart TD
    web["🌐 Web Interface"] -->|uploads code| code-server[("💾 Code Server")]
    code-server -->|highlight data| web
    code-server -->|downloads code| bot-server["💻 Bot Server"]
    bot-server -->|highlight data| code-server
    bot-server -->|motor instructions| robot["🤖 Robot\n(this)"]
    robot -->|sensor data| bot-server
    tag-server["📷 Tag Server"] -->|apriltag positions| bot-server
    
    style robot stroke-width:2px,stroke-dasharray: 5 5,stroke:#3b82f6
    
    click web "https://github.com/AutoSumo/web"
    click code-server "https://github.com/AutoSumo/code-server"
    click bot-server "https://github.com/AutoSumo/server"
    click tag-server "https://github.com/AutoSumo/tag-server"
```

https://user-images.githubusercontent.com/26680599/185769401-c024f6a5-496c-4001-9858-a5e7ccef9a41.mp4

### Hardware

 - ESP32 used for communication and processing
 - Dual H-Bridge motor driver chosen for size and efficiency
 - Back-mounted rechargable battery
 - Two DC motors for drive
 - Two infrared line sensors mounted to the front of the robot
 - Single-point LIDAR mounted to a servo with a 180 degree range of motion
 - Two infrared tachometers (one for each wheel)

Robot body is fully 3D printed and assembled using M3 screws and nuts. Designed using Fusion 360.

[![Robot body in Fusion 360](https://user-images.githubusercontent.com/26680599/186747566-b6fe78d5-234d-42a8-a620-afc675234ee1.png)](./3d/)
![Picture of the robot](https://user-images.githubusercontent.com/26680599/186748286-b1a7dc94-7e34-44f7-9d12-4bc15a54bf48.jpg)
![Picture of the underside of the robot](https://user-images.githubusercontent.com/26680599/186748291-354b9bca-f11a-4f92-94e0-9022a0ad5487.jpg)