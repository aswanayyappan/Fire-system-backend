require("dotenv").config();
const express = require("express");
const axios = require("axios");
const admin = require("firebase-admin");

const app = express();
app.use(express.json());

// ===== CONFIG =====
const BOT_TOKEN = process.env.BOT_TOKEN;
const CHAT_ID = process.env.CHAT_ID;

// Firebase service account JSON file
const serviceAccount = require("./serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://fire-alert-system-b056f-default-rtdb.firebaseio.com"
});

const db = admin.database();

// ===== STATE =====
let lastState = "SAFE";
let lastAlertTime = 0;
const COOLDOWN = 15000; // 15 sec
let isInitialLoad = true;

// ===== TELEGRAM =====
async function sendTelegram(message) {
  try {
    await axios.post(`https://api.telegram.org/bot${BOT_TOKEN}/sendMessage`, {
      chat_id: CHAT_ID,
      text: message,
    });
    console.log("[TELEGRAM] Sent");
  } catch (err) {
    console.log("[TELEGRAM ERROR]", err.message);
  }
}

// ===== ALERT HANDLER =====
async function handleAlert(data, source) {
  const now = Date.now();
  const state = data.alert ? "DANGER" : "SAFE";

  console.log(`[ALERT via ${source}]`, data);

  if (state === "DANGER" && lastState === "SAFE") {

    if (now - lastAlertTime < COOLDOWN) {
      console.log("[SKIP] Cooldown active");
      return;
    }

    const message = `
🔥 FIRE ALERT (${source}) 🔥



Temperature is very high
Smoke: ${data.smoke}

Status: DANGER

`;

    await sendTelegram(message);

    lastAlertTime = now;
  }

  // Update state so we don't keep firing
  lastState = state;
}

// ===== FIREBASE LISTENER =====
db.ref("devices/device_1").on("value", async (snapshot) => {
  const data = snapshot.val();
  
  if (!data) return;

  if (isInitialLoad) {
    console.log("[INIT] Initial database state loaded. Current alert state is:", data.alert);
    isInitialLoad = false;
    lastState = data.alert ? "DANGER" : "SAFE";
    return; // Don't trigger an alert on the initial startup
  }

  await handleAlert(data, "Firebase");
});

// ===== ROUTES =====

// Wake endpoint (fast response)
app.get("/wake", (req, res) => {
  console.log("[WAKE] Server awakened");
  res.send("OK");
});

// Alert endpoint (ESP sends here)
app.post("/alert", async (req, res) => {
  await handleAlert(req.body, "HTTP");
  res.send("OK");
});

app.get("/", (req, res) => {
  res.send("Server Running");
});

// ===== START =====
app.listen(3000, () => {
  console.log("Server running on port 3000");
});
