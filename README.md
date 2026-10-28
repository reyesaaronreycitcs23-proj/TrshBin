# SMART TRASH BIN MONITORING AND CONTROL SYSTEM USING ESP32

## Project Description

The *Smart Trash Bin Monitoring and Control System Using ESP32* is a smart waste management project designed to make trash disposal more convenient and hygienic. It uses an ESP32 microcontroller, ultrasonic sensors, a servo motor, and an LCD to automatically control the trash bin lid and monitor its fill level.

## Objectives

- To develop an automatic trash bin using ESP32.
- To reduce direct contact with the trash bin lid.
- To monitor the waste level inside the bin.
- To display the bin's current status using an LCD.
- To apply IoT-related technology to a practical waste management system.

## System Features

- *Automatic Lid Control:* Opens when an object is detected near the bin.
- *Waste-Level Monitoring:* Measures the distance between the waste and the sensor.
- *LCD Display:* Shows the estimated fill level and bin status.
- *Automatic Closing:* Closes the lid after a short delay.
- *Sensor Error Detection:* Displays an error message when a sensor reading is invalid.

## Components and Materials

- ESP32 development board
- 2 ultrasonic sensors
- Servo motor
- 16x2 LCD with I2C module
- Jumper wires
- Breadboard or suitable connection adapter
- USB cable
- Trash bin and mounting materials
- Computer or laptop

## Technologies and Tools Used

- *Arduino IDE* – Used to write and upload the program to the ESP32.
- *ESP32* – Controls the sensors, servo motor, and LCD.
- *Ultrasonic Sensors* – Detect nearby objects and measure the waste level.
- *Servo Motor* – Opens and closes the trash bin lid.
- *I2C LCD* – Displays the bin's current status.

## System Operation

### 1. Object Detection

The outside ultrasonic sensor detects an object within approximately 5 cm of the bin. When an object is detected, the ESP32 commands the servo motor to open the lid to approximately 90 degrees.

### 2. Automatic Lid Closing

The lid remains open for approximately 3 seconds before the servo motor returns it to the closed position at 0 degrees.

### 3. Waste-Level Monitoring

The inside ultrasonic sensor measures the distance between the sensor and the waste. Using a reference height of 30 cm, the ESP32 estimates the bin's fill level.

### 4. LCD Status Display

The LCD displays the estimated fill level and corresponding status.

| Fill Level | Status |
|---|---|
| Below 30% | LOW |
| 30%–69% | HALF |
| 70%–99% | ALMOST FULL |
| At the full threshold | FULL |
| Invalid sensor reading | SENSOR ERROR |

## Installation and Setup

1. Install the Arduino IDE on your computer.
2. Install the required ESP32 board support in the Arduino IDE.
3. Connect the ultrasonic sensors, servo motor, and LCD to the ESP32 according to your circuit diagram.
4. Open the project's Arduino source code.
5. Select the correct ESP32 board and communication port.
6. Install any libraries required by the source code.
7. Upload the program to the ESP32.
8. Power the system and test the automatic lid, waste-level monitoring, and LCD display.

*Note:* Follow your actual circuit diagram and source code for the correct ESP32 model, GPIO pin assignments, and required libraries. Ensure that the sensors and servo are powered and connected appropriately for the hardware being used.

## Testing and Expected Results

The system should demonstrate the following results:

- The lid opens when an object is detected within the configured distance.
- The lid closes automatically after the specified delay.
- The inside sensor measures the distance to the waste.
- The LCD displays the appropriate waste-level status.
- An error message appears when a sensor reading is invalid.

## Scope and Limitations

The project focuses on automatic lid control and waste-level monitoring using an ESP32 microcontroller. Its performance depends on proper sensor placement, reliable wiring, and an adequate power supply. Incorrect sensor readings or hardware problems may affect its operation.

The current version focuses on the ESP32-based hardware system. A Blynk cloud dashboard is not included in this version unless it is added back to the project's implementation.

## Project Files and Images

The GitHub repository contains the following files:

- ESP32 Arduino IDE source code (.ino)
- Circuit diagram
- Prototype

Upload the actual files and images to the repository and update this section with their correct filenames or links.

## Project Members

*Sections:* BSIT – 411 and BSIT – 412

1. Alicpala, Johanan S.
2. Calingo, Mary Rose M.
3. Jaralve, Khen G.
4. Latoja, Aryanna Claire E.
5. Medrano, Lawrence M.
6. Pangahin, Marijess S.
7. Perez, Trishia Yvone G.
8. Reyes, Aaron Rey M.
9. Reyes, Jade Andrei A.

## Instructor

*Ms. Darlene Opeña*

## Reference Video

[Watch the project reference video on YouTube](https://youtu.be/fDU8Np1jsiY)

## Conclusion

The Smart Trash Bin Monitoring and Control System Using ESP32 demonstrates how a microcontroller, ultrasonic sensors, a servo motor, and an LCD can work together to improve waste disposal. The system provides automatic lid control and waste-level monitoring, helping make trash disposal more convenient and hygienic.
