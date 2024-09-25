#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <PZEM004Tv30.h>
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#define PZEM_SERIAL Serial2
#define CONSOLE_SERIAL Serial
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
//Sesuaikan 
const char* ssid = "POCO F4";
const char* password = "Incrusio";
const char* host = "192.168.209.81:8000";
const char* code = "PZEM02";

HardwareSerial pzemSerial(2);  // Use Serial2 on ESP32



float voltage1, current1, power1, energy1, frequency1, pf1, va1, VAR1;

float zeroIfNan(float v) {
  if (isnan(v)) {
    v = 0;
  }
  return v;
}

void setup() {
  Serial.begin(115200);
  pzemSerial.begin(9600, SERIAL_8N1, 17, 16);  // RX, TX pins for Serial2 on ESP32

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void hasil() {
  voltage1 = pzem.voltage();
  voltage1 = zeroIfNan(voltage1);
  current1 = pzem.current();
  current1 = zeroIfNan(current1);
  power1 = pzem.power();
  power1 = zeroIfNan(power1);
  energy1 = pzem.energy() / 1000;  // kWh
  energy1 = zeroIfNan(energy1);
  frequency1 = pzem.frequency();
  frequency1 = zeroIfNan(frequency1);

  pf1 = pzem.pf();
  pf1 = zeroIfNan(pf1);
  if (pf1 == 0) {
    va1 = 0;
  } else {
    va1 = power1 / pf1;
  }
  if (pf1 == 0) {
    VAR1 = 0;
  } else {
    VAR1 = power1 / pf1 * sqrt(1 - sq(pf1));
  }

  Serial.println("");
  Serial.printf("Voltage        : %.2f V\n", voltage1);
  Serial.printf("Current        : %.2f A\n", current1);
  Serial.printf("Power Active   : %.2f W\n", power1);
  Serial.printf("Frequency      : %.2f Hz\n", frequency1);
  Serial.printf("Cosine Phi     : %.2f PF\n", pf1);
  Serial.printf("Energy         : %.2f kWh\n", energy1);
  Serial.printf("Apparent Power : %.2f VA\n", va1);
  Serial.printf("Reactive Power : %.2f VAR\n", VAR1);
  Serial.printf("---------- END ----------");
  Serial.println("");
}

void loop() {
  delay(24000);
  hasil();
  kirimVolt();
  kirimPower();
  kirimCurrent();
  yield();
}

void kirimData(const char* parameter, float value) {
  WiFiClient client;
  const int httpPort = 80;

  // Uji koneksi
  if (!client.connect(host, httpPort)) {
    Serial.println("Gagal Koneksi ke Server");
    return;
  }

  String link = "http://" + String(host) + "/api/" + parameter + "?code=" + String(code) + "&" + parameter + "=" + String(value);
  delay(2000);

  HTTPClient http;
  // Kirim data
  http.begin(client, link);
  int httpResponseCode = http.GET();

  // Tangkap respon kirim data
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void kirimVolt() {
  hasil();
  kirimData("voltage", voltage1);
}

void kirimPower() {
  hasil();
  kirimData("power", power1);
}

void kirimCurrent() {
  hasil();
  kirimData("current", current1);
}
