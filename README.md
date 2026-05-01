# 🔥 Fire Alert System

A robust real-time fire detection and alert system using an ESP32, Firebase Realtime Database, and a Node.js backend. The system continuously monitors temperature and smoke levels and dispatches an instant Telegram notification if a danger threshold is crossed.

## 🚀 Features

- **Real-Time Monitoring:** ESP32 continuously sends analog sensor data to Firebase.
- **Node.js Observer:** A background service listens to Firebase database changes.
- **Telegram Integration:** Instantly sends out a Telegram message to a specified chat when danger is detected.
- **Anti-Spam State Control:** Only sends an alert when the status transitions from `SAFE` to `DANGER`.

## 📂 Project Structure

- `fire.ino` - The ESP32 Arduino sketch that handles sensor reading, threshold calibration, and writing to Firebase.
- `server/` - The Node.js backend that listens to Firebase and triggers Telegram alerts.

## 🛠️ Setup & Installation

### 1. Hardware Setup (Arduino)
- Configure your WiFi `SSID` and `PASSWORD` in `fire.ino`.
- Flash the code to your ESP32.

### 2. Backend Setup (Node.js)
```bash
cd server
npm install
```

### 3. Environment Variables
Create a `.env` file in the `server` directory and add your Telegram bot credentials:
```env
BOT_TOKEN=your_telegram_bot_token
CHAT_ID=your_chat_id
```

### 4. Firebase Service Account
1. Go to your Firebase Console -> Project Settings -> Service Accounts.
2. Generate a new private key.
3. Save the downloaded JSON file as `serviceAccountKey.json` inside the `server/` directory.

## 🏃‍♂️ Running the Server

Start the Node.js observer:
```bash
node server.js
```

You should see `Server running on port 3000` in the terminal, indicating that the backend is actively listening for anomalies.
