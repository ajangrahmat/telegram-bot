#include <AViShaWiFi.h>
#include <ArduinoJson.h>
#include <AsyncTelegram2.h>
#include <time.h>
#define MYTZ "CET-1CEST,M3.5.0,M10.5.0/3"

#ifdef ESP8266
#include <ESP8266WiFi.h>
BearSSL::WiFiClientSecure client;
BearSSL::Session session;
BearSSL::X509List certificate(telegram_cert);
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClientSecure.h>
WiFiClientSecure client;
#endif

AsyncTelegram2 myBot(client);

const char *ssid = "Kelas Robot";
const char *pass = "kumaha aa we";
const char *url = "https://ai.sumopod.com/v1/chat/completions";
const char *token = "8864111452:AAGC6_zNN2sd1Ny9u0HesglVUE-HhCeHuK4";
int64_t userid = 262249300; // isi dengan user id telegram anda, bisa diambil
                             // dari bot @userinfobot

AViShaWiFi wifi;

String persona = "Kamu adalah seorang petani Aquaponik yang ahli dalam menanam "
                 "sayuran dan ikan. Jangan pake ###, ** dan simbol lainnya, plain teks aja";

int sensorKelembaban = 68;
int sensorSuhu = 25;

String prompt = "Tolong cek kondisi aquaponik saya dong!";

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  wifi.begin(ssid, pass);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(500);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }

#ifdef ESP8266
  // Sync time with NTP, to check properly Telegram certificate
  configTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  // Set certficate, session and some other base client properies
  client.setSession(&session);
  client.setTrustAnchors(&certificate);
  client.setBufferSizes(1024, 1024);
#elif defined(ESP32)
  // Sync time with NTP
  configTzTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  client.setCACert(telegram_cert);
#endif

  // Set the Telegram bot properies
  myBot.setUpdateTime(2000);
  myBot.setTelegramToken(token);

  // Enable insecure fallback (optional, but recommended for better reliability)
  myBot.enableInsecureFallback();

  // Check if all things are ok
  Serial.print("\nTest Telegram connection... ");
  bool connected = myBot.begin();
  const char *connection_mode = myBot.getConnectionModeName();
  connected ? Serial.println("OK") : Serial.println("NOK");
  Serial.printf("Connection mode: %s\n", connection_mode);

  char welcome_msg[192];
  snprintf(welcome_msg, sizeof(welcome_msg),
           "BOT @%s online\nConnection mode: %s\n/help all commands avalaible.",
           myBot.getBotName(), connection_mode);

  // Send a message to specific user who has started your bot
  myBot.sendTo(userid, welcome_msg);
}

void loop() {

  static uint32_t ledTime = millis();
  if (millis() - ledTime > 150) {
    ledTime = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  // local variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {

    // Send a message to your public channel
    String message;
    message += "Message from @";
    message += myBot.getBotName();
    message += ":\n";
    message += msg.text;
    Serial.println(message);

    prompt = msg.text;

    std::vector<String> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Accept: application/json");
    headers.push_back("Authorization: Bearer sk-T9EW9L2Vym7xzbdIEgbxoQ");

    // ArduinoJson
    String json;
    JsonDocument doc;
    doc["model"] = "gpt-4o-mini";
    JsonObject messages_0 = doc["messages"].add<JsonObject>();
    messages_0["role"] = "user";
    messages_0["content"] = persona + "\n\n" + prompt + "\n\n" +
                            "Sensor Kelembaban: " + String(sensorKelembaban) +
                            "\n" + "Sensor Suhu: " + String(sensorSuhu);
    doc["max_tokens"] = 300;
    doc["temperature"] = 0.7;
    doc.shrinkToFit();
    serializeJson(doc, json);

    String response = wifi.httpsPOST(url, json, headers);
    Serial.println(response);

    // Stream& input;
    JsonDocument doc2;
    DeserializationError error = deserializeJson(doc2, response);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject choices_0 = doc2["choices"][0];
    const char *choices_0_finish_reason = choices_0["finish_reason"]; // "stop"
    int choices_0_index = choices_0["index"];                         // 0

    JsonObject choices_0_message = choices_0["message"];
    const char *choices_0_message_content =
        choices_0_message["content"]; // "Greetings, Earth dweller! 🌍✨ ...
    const char *choices_0_message_role =
        choices_0_message["role"]; // "assistant"

    Serial.println(choices_0_message_content);

    String pesanBalasan = String(choices_0_message_content);

    myBot.sendTo(msg.sender.id, pesanBalasan);
  }

}