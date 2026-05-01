require("dotenv").config();
const express = require("express");
const axios = require("axios");
const admin = require("firebase-admin");

const app = express();

// ===== CONFIG =====
const BOT_TOKEN = process.env.BOT_TOKEN;
const CHAT_ID = process.env.CHAT_ID;

// Firebase service account JSON file
// Ensure you have a serviceAccountKey.json file in this directory.
const serviceAccount = require("./serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://fire-alert-system-b056f-default-rtdb.firebaseio.com"
});

const db = admin.database();

// ===== STATE CONTROL (anti-spam) =====
let lastAlertState = false;

// ===== TELEGRAM SEND =====
async function sendTelegram(message) {
  const url = `https://api.telegram.org/bot${BOT_TOKEN}/sendMessage`;

  try {
    await axios.post(url, {
      chat_id: CHAT_ID,
      text: message
    });

    console.log("[TELEGRAM] Sent");
  } catch (err) {
    console.log("[TELEGRAM ERROR]", err.message);
  }
}

// ===== FIREBASE LISTENER =====
db.ref("devices/device_1").on("value", async (snapshot) => {
  const data = snapshot.val();

  if (!data) return;

  console.log("[DATA]", data);

  const isDanger = data.alert === true;

  // Trigger only when SAFE → DANGER
  if (isDanger && !lastAlertState) {

    const message = `
🔥 FIRE ALERT 🔥

Temperature is very high
Smoke: ${data.smoke}

Status: DANGER
`;

    await sendTelegram(message);
  }

  lastAlertState = isDanger;
});

// ===== SERVER =====
app.get("/", (req, res) => {
  res.send("Server Running");
});

app.listen(3000, () => {
  console.log("Server running on port 3000");
});
