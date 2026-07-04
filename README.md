# RASD - Recognition and Surveillance Device

An IoT-based parking authorization and surveillance system built with Arduino Uno and ESP32-CAM. RASD verifies vehicle/personnel access using RFID authentication, automatically detects unauthorized entry attempts, and sends real-time photo alerts via Telegram — enabling remote, hands-free monitoring of restricted parking areas.

## 🎯 Project Overview

Unauthorized parking and unmonitored access points are common security challenges in shared facilities. RASD addresses this by combining RFID-based access control with automated visual verification, giving administrators instant visibility into who enters a restricted area — without needing an on-site guard.

This project was developed as part of a university course during our Electrical Engineering studies at King Abdulaziz University.

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

Developed by a team of 6 students as part of a university course project.

## 📸 Demo

*(Add your photo/video here — you can drag and drop the image/video file directly into this README on GitHub, or paste a YouTube link if you uploaded a demo video)*

## 🚀 Future Improvements

- Mobile app integration for real-time access logs
- Facial recognition as a secondary verification layer
- Cloud-based dashboard for historical violation records

---
**Author:** Abdulaziz S. Alqahtani
