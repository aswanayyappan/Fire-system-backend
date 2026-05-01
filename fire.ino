#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ================= WIFI =================
#define WIFI_SSID "main"
#define WIFI_PASSWORD "12345678"

// ================= FIREBASE =================
#define FIREBASE_HOST "fire-alert-system-b056f-default-rtdb.firebaseio.com"
#define DEVICE_PATH "/devices/device_1"

// ================= PINS =================
#define TEMP_PIN 34
#define SMOKE_PIN 35
#define BUZZER 5

WiFiClientSecure client;

// ================= FILTER =================
int readAnalog(int pin) {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / 10;
}

// ================= CALIBRATION =================
// Adjusted for realistic output
float convertTemp(int raw) {
  return (raw - 300) * 0.3;   // ~25–45°C range
}

float convertSmoke(int raw) {
  return (raw - 1200) * 0.1;  // relative smoke scale
}

// ================= FIREBASE SEND =================
void sendToFirebase(String json) {
  HTTPClient https;
  client.setInsecure();

  String url = "https://" + String(FIREBASE_HOST) + DEVICE_PATH + ".json";

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");

    int code = https.PUT(json);

    Serial.print("[HTTP] Code: ");
    Serial.println(code);

    if (code <= 0) {
      Serial.println("[HTTP] ERROR: " + https.errorToString(code));
    }

    https.end();
  } else {
    Serial.println("[HTTP] BEGIN FAILED");
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SYSTEM BOOT ===");

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // WiFi
  Serial.print("[WiFi] Connecting...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Connected");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());

  // Initial Firebase write
  sendToFirebase("{\"status\":\"INIT\",\"temperature\":0,\"smoke\":0,\"alert\":false}");
}

// ================= LOOP =================
void loop() {

  int rawTemp = readAnalog(TEMP_PIN);
  int rawSmoke = readAnalog(SMOKE_PIN);

  float temp = convertTemp(rawTemp);
  float smoke = convertSmoke(rawSmoke);

  // Debug logs
  Serial.print("[RAW] Temp=");
  Serial.print(rawTemp);
  Serial.print(" | Smoke=");
  Serial.println(rawSmoke);

  Serial.print("[DATA] Temp=");
  Serial.print(temp);
  Serial.print(" °C | Smoke=");
  Serial.println(smoke);

  // ===== ALERT LOGIC =====
  bool danger = (temp > 45 || smoke > 50);

  Serial.print("[STATE] ");
  Serial.println(danger ? "DANGER" : "SAFE");

  digitalWrite(BUZZER, danger);

  // ===== JSON =====
  String json = "{";
  json += "\"temperature\":" + String(temp) + ",";
  json += "\"smoke\":" + String(smoke) + ",";
  json += "\"status\":\"" + String(danger ? "DANGER" : "SAFE") + "\",";
  json += "\"alert\":" + String(danger ? "true" : "false");
  json += "}";

  // ===== SEND EVERY CYCLE =====
  sendToFirebase(json);

  delay(2000);
}