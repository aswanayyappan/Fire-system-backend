#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// WIFI
#define WIFI_SSID "main"
#define WIFI_PASSWORD "12345678"

// RENDER SERVER
#define SERVER_URL "https://your-app.onrender.com"

// FIREBASE
#define FIREBASE_HOST "fire-alert-system-b056f-default-rtdb.firebaseio.com"
#define DEVICE_PATH "/devices/device_1"

// PINS
#define TEMP_PIN 34
#define SMOKE_PIN 35
#define BUZZER 5

WiFiClientSecure client;

// ===== FILTER =====
int readAnalog(int pin) {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / 10;
}

// ===== CALIBRATION =====
float convertTemp(int raw) {
  return (raw - 300) * 0.3;
}

float convertSmoke(int raw) {
  return (raw - 1200) * 0.1;
}

// ===== FIREBASE =====
void sendToFirebase(String json) {
  HTTPClient https;
  client.setInsecure();

  String url = "https://" + String(FIREBASE_HOST) + DEVICE_PATH + ".json";

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.PUT(json);
    https.end();
  }
}

// ===== WAKE SERVER =====
void wakeServer() {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String url = String(SERVER_URL) + "/wake";

  if (http.begin(client, url)) {
    int code = http.GET();
    Serial.print("[WAKE] ");
    Serial.println(code);
    http.end();
  }
}

// ===== SEND ALERT =====
void sendAlert(float temp, float smoke) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String url = String(SERVER_URL) + "/alert";

  String json = "{";
  json += "\"alert\":true,";
  json += "\"temperature\":" + String(temp) + ",";
  json += "\"smoke\":" + String(smoke);
  json += "}";

  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(json);
    Serial.print("[ALERT] ");
    Serial.println(code);
    http.end();
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println("WiFi Connected");

  sendToFirebase("{\"status\":\"INIT\"}");
}

// ===== LOOP =====
void loop() {

  int rawTemp = readAnalog(TEMP_PIN);
  int rawSmoke = readAnalog(SMOKE_PIN);

  float temp = convertTemp(rawTemp);
  float smoke = convertSmoke(rawSmoke);

  bool danger = (temp > 45 || smoke > 120);

  digitalWrite(BUZZER, danger);

  // Send data to Firebase
  String json = "{";
  json += "\"temperature\":" + String(temp) + ",";
  json += "\"smoke\":" + String(smoke) + ",";
  json += "\"alert\":" + String(danger ? "true" : "false");
  json += "}";

  sendToFirebase(json);

  // Wake + Alert logic
  static bool lastDanger = false;

  if (danger && !lastDanger) {
    Serial.println("[EVENT] FIRE DETECTED");

    wakeServer();      // Step 1
    delay(8000);       // Step 2 (wait for Render)

    sendAlert(temp, smoke); // Step 3
  }

  lastDanger = danger;

  delay(2000);
}