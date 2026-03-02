#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "secrets.h"

// ---------- PIN SETTINGS (your friend can change later) ----------
const int FLAME_PIN  = 34;   // Flame sensor D0/OUT -> GPIO34
const int BUZZER_PIN = 23;   // Buzzer -> GPIO23

// Most flame sensor modules are ACTIVE-LOW.
// If your sensor gives HIGH when flame detected, change this to false.
const bool FLAME_ACTIVE_LOW = true;

// ---------- Anti-spam (Telegram cooldown) ----------
unsigned long lastAlertMs = 0;
const unsigned long ALERT_COOLDOWN_MS = 60UL * 1000UL; // 60 seconds

WiFiClientSecure securedClient;
UniversalTelegramBot bot(BOT_TOKEN, securedClient);

bool alarmOn = false;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void buzzerOn()  { alarmOn = true;  digitalWrite(BUZZER_PIN, HIGH); }
void buzzerOff() { alarmOn = false; digitalWrite(BUZZER_PIN, LOW);  }

bool isFlameDetected() {
  int v = digitalRead(FLAME_PIN);
  if (FLAME_ACTIVE_LOW) return (v == LOW);
  else                  return (v == HIGH);
}

void sendTelegramOncePerCooldown(const String &msg) {
  unsigned long now = millis();
  if (now - lastAlertMs < ALERT_COOLDOWN_MS) return;
  lastAlertMs = now;
  bot.sendMessage(CHAT_ID, msg, "");
}

void setup() {
  Serial.begin(115200);

  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerOff();

  // Telegram uses HTTPS.
  // Easy method for student projects (no certificate handling).
  securedClient.setInsecure();

  connectWiFi();

  bot.sendMessage(CHAT_ID, "✅ Fire Detection System (ESP32) is online.", "");
}

void loop() {
  // Reconnect WiFi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  bool flame = isFlameDetected();

  if (flame && !alarmOn) {
    buzzerOn();
    sendTelegramOncePerCooldown("🔥 ALERT: Flame detected! Buzzer ON.");
  }

  if (!flame && alarmOn) {
    buzzerOff();
    sendTelegramOncePerCooldown("✅ Update: Flame cleared. Buzzer OFF.");
  }

  delay(200);
}