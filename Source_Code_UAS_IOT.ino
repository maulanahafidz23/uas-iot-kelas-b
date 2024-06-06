#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

const int lampuMerah = 2; // Pin untuk lampu merah
const int lampuHijau = 4; // Pin untuk lampu Hijau
const int lampuBiru = 14; // Pin untuk lampu Biru
const int lampuUngu = 12; // Pin untuk lampu Ungu

// Ganti dengan SSID dan Password WiFi Anda
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com"; // broker

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// URL dan API Key dari Antares Pak Anton
const char* serverName = "https://platform.antares.id:8443/~/antares-cse/antares-id/AQWM/weather_airQuality_nodeCore_teknik/la";
const char* apiKey = "ee8d16c4466b58e1:3b8d814324c84c89";

// Fungsi untuk menerima data
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("");
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  String data = ""; // variabel untuk menyimpan data yang berbentuk array char
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i]; // menyimpan kumpulan char kedalam string
  }
  if (data == "1") {
    digitalWrite(lampuBiru, HIGH);
  } else if (data == "5") {
    digitalWrite(lampuBiru, LOW);
  } else if (data == "2") {
    digitalWrite(lampuHijau, HIGH);
  } else if (data == "6") {
    digitalWrite(lampuHijau, LOW);
  }  else if (data == "3") {
    digitalWrite(lampuUngu, HIGH);
  } else if (data == "7") {
    digitalWrite(lampuUngu, LOW);
  } else if (data == "4") {
    digitalWrite(lampuMerah, HIGH);
  } else if (data == "8") {
    digitalWrite(lampuMerah, LOW);
  } else if (data == "0") {
    displaySensorData();
  }
}

// fungsi untuk mengubungkan ke broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      displaySensorData();
      client.subscribe("IOTB/3/LED");
      client.subscribe("IOTB/3/SUHU");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(lampuMerah, OUTPUT); // Mengatur pin lampu sebagai output
  pinMode(lampuHijau, OUTPUT); // Mengatur pin lampu sebagai output
  pinMode(lampuBiru, OUTPUT); // Mengatur pin lampu sebagai output
  pinMode(lampuUngu, OUTPUT); // Mengatur pin lampu sebagai output

  // Menghubungkan ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" connected!");

  // Menghubungkan Ke MQTT
  client.setServer(mqtt_server, 1883); // setup awal ke server mqtt
  client.setCallback(callback);
  randomSeed(micros());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Fungsi untuk menampilkan data sensor
void displaySensorData() {
  // Mengecek status koneksi WiFi
  if (WiFi.status() == WL_CONNECTED) {
    // Membuat objek HTTPClient untuk melakukan request HTTP
    HTTPClient http;
    // Memulai koneksi HTTP ke server yang ditentukan oleh serverName
    http.begin(serverName);
    // Menambahkan header API key untuk otentikasi
    http.addHeader("X-M2M-Origin", apiKey);
    // Menambahkan header untuk menerima respons dalam format JSON
    http.addHeader("Accept", "application/json");

    // Melakukan HTTP GET request
    int httpResponseCode = http.GET();

    // Mengecek kode respons dari server
    if (httpResponseCode > 0) {
      // Mendapatkan payload atau data respons dari server
      String payload = http.getString();
      // Serial.println(payload);
      
      // Membuat dokumen JSON dinamis untuk menampung data dari payload
      DynamicJsonDocument doc(1024);
      // Mengurai (deserialisasi) payload JSON ke dalam dokumen JSON
      DeserializationError error = deserializeJson(doc, payload);

      // Mengecek apakah deserialisasi berhasil
      if (!error) {
        // Mengambil objek "m2m:cin" dari dokumen JSON
        JsonObject cinObj = doc["m2m:cin"];
        // Mengambil data "con" sebagai string
        String conData = cinObj["con"];

        // Membuat dokumen JSON statis untuk menampung data "con"
        StaticJsonDocument<256> conDoc;
        // Mengurai (deserialisasi) string "con" JSON ke dalam dokumen JSON
        DeserializationError conError = deserializeJson(conDoc, conData);

        // Mengecek apakah deserialisasi data "con" berhasil
        if (!conError) {
          // Mengambil dan mengonversi data suhu dari dokumen JSON
          float temperature = conDoc["Temp"].as<float>();
          // Mengambil dan mengonversi data kelembaban dari dokumen JSON
          //float humidity = conDoc["Hum"].as<float>();
          // Mengambil dan mengonversi data ozon dari dokumen JSON
          float ozon = conDoc["Ozon"].as<float>();
          // Mengambil dan mengonversi data UV dari dokumen JSON
          //float uv = conDoc["UV"].as<float>();
          // Mengambil dan mengonversi data AQI dari dokumen JSON
          float aqi = conDoc["AQI"].as<float>();
          // Mengambil dan mengonversi data PM2.5 dari dokumen JSON
          float pm25 = conDoc["PM2.5"].as<float>();
          // Mengambil dan mengonversi data PM10 dari dokumen JSON
          float pm10 = conDoc["PM10"].as<float>();
          // Mengambil dan mengonversi data NO2 dari dokumen JSON
          float no2 = conDoc["NO2"].as<float>();
          // Mengambil dan mengonversi data CO dari dokumen JSON
          float co = conDoc["CO"].as<float>();

          // Mencetak data sensor ke Serial Monitor
          Serial.println("Data sensor:");
          Serial.print("Suhu : ");
          Serial.println(temperature, 2);  // Mencetak suhu dengan 2 angka desimal
          Serial.print("Ozon : ");
          Serial.println(ozon, 2);  // Mencetak ozon dengan 2 angka desimal
          Serial.print("AQI : ");
          Serial.println(aqi, 2);
          Serial.print("PM2.5 : ");
          Serial.println(pm25, 2);
          Serial.print("PM10 : ");
          Serial.println(pm10, 2);
          Serial.print("NO2 : ");
          Serial.println(no2, 2);
          Serial.print("CO : ");
          Serial.println(co, 2);

          // Publish data sensor ke broker MQTT
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", temperature);
          client.publish("IOTB/3/SUHU", msg);
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", aqi);
          client.publish("IOTB/3/AQI", msg);
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", pm25);
          client.publish("IOTB/3/PM2.5", msg);
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", pm10);
          client.publish("IOTB/3/PM10", msg);
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", ozon);
          client.publish("IOTB/3/Ozon", msg);
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", no2);
          client.publish("IOTB/3/NO2", msg);
          snprintf (msg, MSG_BUFFER_SIZE, "%.1f", co);
          client.publish("IOTB/3/CO", msg);
        } else {
          // Mencetak pesan error jika terjadi kesalahan saat mengurai data "con"
          Serial.print("Error parsing 'con' JSON: ");
          Serial.println(conError.c_str());
        }
      } else {
        // Mencetak pesan error jika deserialisasi payload JSON gagal
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }
    } else {
      // Mencetak pesan error jika terjadi kesalahan pada HTTP request
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }

    // Mengakhiri koneksi HTTP
    http.end();
  }
}