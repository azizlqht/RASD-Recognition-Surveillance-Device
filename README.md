# RASD - Recognition and Surveillance Device

An IoT-based parking authorization and surveillance system built with Arduino Uno and ESP32-CAM. RASD verifies vehicle/personnel access using RFID authentication, automatically detects unauthorized entry attempts, and sends real-time photo alerts via Telegram — enabling remote, hands-free monitoring of restricted parking areas.

## 🎯 Project Overview

Unauthorized parking and unmonitored access points are common security challenges in shared facilities. RASD addresses this by combining RFID-based access control with automated visual verification, giving administrators instant visibility into who enters a restricted area — without needing an on-site guard.

This project was developed as part of a IE-201 course during our Electrical Engineering studies at King Abdulaziz University.

## ⚙️ How It Works

1. A user presents an RFID tag/wristband at the entry point.
2. The Arduino Uno reads the RFID tag and checks it against a list of authorized IDs.
3. If **authorized** → access is granted (e.g., gate/signal opens).
4. If **unauthorized** → the ESP32-CAM automatically captures a photo of the violation.
5. The captured image is instantly sent to a **Telegram bot**, notifying administrators in real time with visual proof of the unauthorized attempt.


## 🛠️ Components & Technologies

- Arduino Uno
- ESP32-CAM Module
- RFID Reader + Tags/Wristbands
- Telegram Bot API
- Arduino IDE (C/C++)

## 👥 Team
1-Abdulelah Al-Qahtani 
2-Ibrahim Al-Sulami
3-Abdulaziz Al-Qahtani
4-Yazan Assiri
5-Abdulmehsen Al-Bladi
6-Ibrahim Al-Noger

## 📸 Demo

▶️ [Watch the full project demo on YouTube](https://youtu.be/3xsCZu3Srio) *(Arabic narration)*

**Full setup**

<img src="https://github.com/user-attachments/assets/28e4055c-a1ef-4bc1-a850-1320929e7b6a" width="450">

**Detecting an unauthorized parking violation**

<img src="https://github.com/user-attachments/assets/8553a7f0-be88-4820-aac4-eda77b031d79" width="450">

**Live violation alert sent via Telegram bot**

<img src="https://github.com/user-attachments/assets/c9cac96a-2caa-4813-a43a-b023ffbb1519" width="450">

**Hardware wiring**

<img src="https://github.com/user-attachments/assets/680f5648-24f2-42d0-875e-fc914ed7bb58" width="450">

**Team at work**

<img src="https://github.com/user-attachments/assets/c99f24a5-c1f9-41a9-bc6a-236776fb750f" width="450">

## 🚀 Future Improvements

- Mobile app integration for real-time access logs
- Facial recognition as a secondary verification layer
- Cloud-based dashboard for historical violation records

---
**Author:** Abdulaziz S. Alqahtani

