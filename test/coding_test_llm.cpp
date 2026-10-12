#include <AViShaWiFi.h>
#include <ArduinoJson.h>

const char *ssid = "isi SSID anda disini";
const char *pass = "isi password wifi anda disini";
const char *url = "https://ai.sumopod.com/v1/chat/completions";

AViShaWiFi wifi;

String persona = "Kamu adalah seorang petani Aquaponik yang ahli dalam menanam "
                 "sayuran dan ikan. Kamu terhubung ke sistem pakan otomatis "
                 "dan juga terdapat beberapa sensor";

int sensorKelambangan = 68;
int sensorSuhu = 25;

String prompt = "Tolong cek kondisi aquaponik saya dong!";

void setup() {
  Serial.begin(115200);
  wifi.begin(ssid, pass);
}

void loop() {
  std::vector<String> headers;
  headers.push_back("Content-Type: application/json");
  headers.push_back("Accept: application/json");
  headers.push_back("Authorization: Bearer isi_api_key_anda_disini");

  // ArduinoJson
  String json;
  JsonDocument doc;
  doc["model"] = "gpt-4o-mini";
  JsonObject messages_0 = doc["messages"].add<JsonObject>();
  messages_0["role"] = "user";
  messages_0["content"] = persona + "\n\n" + prompt + "\n\n" +
                          "Sensor Kelembaban: " + String(sensorKelembaban) +
                          "\n" + "Sensor Suhu: " + String(sensorSuhu);
  doc["max_tokens"] = 150;
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
  const char *choices_0_message_role = choices_0_message["role"]; // "assistant"

  Serial.println(choices_0_message_content);


  delay(10000000);
}