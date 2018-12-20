#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <LoRa.h> // arduino lora library by Sandeep Mistry
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.hpp"
#define LORA_MODE 13 // with some test I decided that this configuration works best in terms of distance / noise immunity / packet send time (~4 sec)

// lora pin definitions
#define RST                   14
#define DIO0                  2
#define SS                    5

HTTPClient http;
bool is_connected = false;

bool is_cnt_sent = true;
int global_cnt = 0;

uint8_t buff[256];
const uint8_t packet_header[] = "caftra:"; // just a way to distinguish our messages
uint16_t packet_header_size;
void onReceive_LoRa(int packetSize) {
  Serial.print("onReceive_LoRa: \"");
  uint8_t off = 0;
  while (LoRa.available() && off < packetSize)
    off += LoRa.readBytes(buff + off, packetSize - off);
  Serial.write(buff, off);
  Serial.println("\"");

  if (off <= packet_header_size) return; // if packet is too small
  for (int i = 0; i < packet_header_size; i++)
    if (buff[i] != packet_header[i]) return; // if there is header mismatch
  
  uint32_t cnt = 0;
  for (int i = packet_header_size; i < off; i++){
    if (buff[i] > '9' || buff[i] < '0') return; // remaining bytes should be digits
    cnt *= 10;
    cnt += buff[i] - '0';
  }

  Serial.print("CORRECT PACKET: ");
  Serial.println(cnt);
  global_cnt = cnt;
  is_cnt_sent = false; // NOTE: this function is interrupt and HTTPClient::GET uses hostByName and it uses tasks and they fail in interrupts
                       //so instead of sending GET here, I put a global flag and sent the request in the main loop
}

void wifi_connect(){
  int try_val = 0;
  const int try_limit = 50; // 25 sec

  Serial.print("Connecting to WiFi ");
  WiFi.mode(WIFI_STA);
  WiFi.begin("WIN-TECH", "qweasd123");
  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECT_FAILED){
      is_connected = false;
      Serial.println("Failed!");

      WiFi.mode(WIFI_OFF);
      return;
    }
    try_val++;
    if (try_val == try_limit){
      is_connected = false;
      Serial.println("Failed!");

      WiFi.mode(WIFI_OFF);
      return;
    }
    Serial.print(". ");
    delay(500);
  }
  is_connected = true;
  Serial.println("OK!");
}

void setup() {
  packet_header_size = strlen((char*)packet_header);

  Serial.begin(115200);
  Serial.println();

  // setup LoRa transceiver module
  Serial.print("Initializing LoRa ");
  LoRa.setPins(SS, RST, DIO0);
  while (!LoRa.begin(433E6)) { // 433E6, 866E6, 915E6
    Serial.print(". ");
    delay(500);
  }
  LoRa.setSyncWord(0xA2); // just another way to distinguish our messages
  LoRa.setTxPower(20); // 2 - 20
  LoRa.setSpreadingFactor(LoraMode<LORA_MODE>::spreadingfactor);
  LoRa.setSignalBandwidth(LoraMode<LORA_MODE>::bandwidth);
  LoRa.setCodingRate4(LoraMode<LORA_MODE>::codingrate);
  LoRa.enableCrc();
  LoRa.onReceive(onReceive_LoRa);
  LoRa.receive();
  Serial.println("OK!");
  
  wifi_connect();
}

void loop() {
  delay(10);
  if (WiFi.status() != WL_CONNECTED) {
    is_connected = false;
    Serial.print("WiFi Disconnected, trying to reconnect ");
    wifi_connect();
  }

  if (!is_cnt_sent && is_connected){ // there is data to send and wifi is connected
    is_cnt_sent = true;

    Serial.println("Sending GET request");
    
    http.begin("http://cildir.gq/cafetraset.php?tra=" + String(global_cnt, 10)); // send a GET request with our data
    int httpCode = http.GET(); // blocking but onReceive_LoRa is interrupt so it can work in the middle of this
    
    // httpCode will be negative on error
    if (httpCode > 0) // HTTP header has been sent and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode); // payload is empty so no need to print it
    else
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());

    http.end();
  }
}
