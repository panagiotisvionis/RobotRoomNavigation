# RobotRoomNavigation 🤖

**Arduino Robot with Dual Functionality**  
This project implements two autonomous behaviors for a mobile robot using an Arduino UNO, ultrasonic sensor, servo motor, and DC motors with an L298N motor driver.

---

## 🚀 Features

1. **Corridor Following (CORRIDOR_FOLLOW):**  
   The robot stays centered between two walls using left and right distance measurements.

2. **Room Navigation and Exit (ROOM_NAVIGATION):**  
   The robot moves inside a room, follows the wall, detects a doorway, and exits through it once the opening is verified.

---

## 🔧 Hardware Requirements

- Arduino UNO  
- 1x HC-SR04 Ultrasonic Sensor  
- 1x Servo Motor (SG90 or similar)  
- 2x DC Motors  
- L298N Motor Driver Module  
- Battery pack (6V–12V recommended)  
- Jumper wires and chassis

---

## 🧠 How It Works

- The **ultrasonic sensor** is mounted on a **servo** which rotates to collect distances from left, center, and right.
- Using this data, the robot adjusts motor speeds to follow the corridor center or detect walls and doorways.
- The behavior is determined by the variable:

```cpp
OperationMode currentMode = ROOM_NAVIGATION;
```

Change it to:

```cpp
OperationMode currentMode = CORRIDOR_FOLLOW;
```

if you want to test the corridor-following behavior instead.

---

## 🛠️ Pin Configuration

| Component           | Arduino Pin |
|--------------------|-------------|
| Left Motor PWM     | 5           |
| Right Motor PWM    | 6           |
| Left Motor DIR     | 2, 4        |
| Right Motor DIR    | 7, 8        |
| Ultrasonic TRIG    | A1          |
| Ultrasonic ECHO    | A0          |
| Servo Signal       | A2          |

---

## 📝 Author

Developed by **Panagiotis Vionis**  
Feel free to fork, modify, or contribute!

---

## 📄 License

This project is open-source and distributed under the MIT License.
