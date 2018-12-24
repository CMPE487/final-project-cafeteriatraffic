#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <LoRa.h> // arduino lora library by Sandeep Mistry
extern "C" {
  #include <user_interface.h> // for entering monitoring mode
}

#include "config.hpp"
#define LORA_MODE 13 // with some test I decided that this configuration works best in terms of distance / noise immunity / packet send time (~4 sec)

// lora pin definitions
#define RST                   16
#define DIO0                  5
#define SS                    15

#define DATA_LENGTH           112

#define TYPE_MANAGEMENT       0x00
#define TYPE_CONTROL          0x01
#define TYPE_DATA             0x02
#define SUBTYPE_PROBE_REQUEST 0x04

uint32_t last_millis = 0;
uint32_t overflow_cnt = 0;
uint32_t get_seconds(){ // millis overflows in around 47 days but this function returns seconds so it will last 1000 times longer
  uint32_t new_millis = millis();
  if (last_millis > new_millis) overflow_cnt++;
  last_millis = new_millis;

  uint32_t current_period_seconds = new_millis / 1000;
  uint32_t real_seconds = current_period_seconds + overflow_cnt * (0xFFFFFFFF / 1000) + 1; // NOTE: this shouldn't be 0
  return real_seconds;
}

struct TimestampedMAC { // 10 or 12 bytes per struct (probably 12 bytes because of aligning)
  uint32_t timestamp; // the struct is considered null if this is 0
  uint8_t mac[6];
};

uint16_t count_of_macs = 0; // how many unique mac in the list that has been received in last 5 minutes
constexpr uint16_t size_limit_of_mac_list = 2500;
TimestampedMAC mac_list[size_limit_of_mac_list];

int16_t find_in_mac_list(uint8_t* mac){ // check if mac is already in the list
  for (uint16_t i = 0; i < size_limit_of_mac_list; i++){
    if (mac_list[i].timestamp == 0) continue; // pass the null ones

    bool found = true;
    for (uint8_t j = 0; j < 6; j++){
      if (mac_list[i].mac[j] != mac[j]){
        found = false; // mismatch
        break;
      }
    }
    if (found) return i;
  }
  return -1; // if not found, return -1
}
void update_timestamp_in_mac_list(uint16_t idx){
  mac_list[idx].timestamp = get_seconds();
}
void add_to_mac_list(uint8_t* mac){ // replace a null entry with the new mac
  if (count_of_macs >= size_limit_of_mac_list) return; // there is no null entry in the list

  for (uint16_t i = 0; i < size_limit_of_mac_list; i++){
    if (mac_list[i].timestamp != 0) continue; // pass the not null ones

    mac_list[i].timestamp = get_seconds();
    for (uint8_t j = 0; j < 6; j++) mac_list[i].mac[j] = mac[j];
    count_of_macs++;
    break;
  }
}
void remove_more_than_5_mins(){ // check all non null entries and if their timestamp is old, make them null
  if (get_seconds() <= 60 * 5) return; // 5 minutes hasn't been passed yet
  uint32_t five_min_ago = get_seconds() - 60 * 5;

  for (uint16_t i = 0; i < size_limit_of_mac_list; i++){
    if (mac_list[i].timestamp == 0) continue;

    if (mac_list[i].timestamp < five_min_ago) {
      mac_list[i].timestamp = 0; // make it null
      count_of_macs--;
    }
  }
}

struct RxControl {
 signed rssi:8; // signal intensity of packet
 unsigned rate:4;
 unsigned is_group:1;
 unsigned:1;
 unsigned sig_mode:2; // 0:is 11n packet; 1:is not 11n packet;
 unsigned legacy_length:12; // if not 11n packet, shows length of packet.
 unsigned damatch0:1;
 unsigned damatch1:1;
 unsigned bssidmatch0:1;
 unsigned bssidmatch1:1;
 unsigned MCS:7; // if is 11n packet, shows the modulation and code used (range from 0 to 76)
 unsigned CWB:1; // if is 11n packet, shows if is HT40 packet or not
 unsigned HT_length:16;// if is 11n packet, shows length of packet.
 unsigned Smoothing:1;
 unsigned Not_Sounding:1;
 unsigned:1;
 unsigned Aggregation:1;
 unsigned STBC:2;
 unsigned FEC_CODING:1; // if is 11n packet, shows if is LDPC packet or not.
 unsigned SGI:1;
 unsigned rxend_state:8;
 unsigned ampdu_cnt:8;
 unsigned channel:4; //which channel this packet in.
 unsigned:12;
};

struct SnifferPacket{
    struct RxControl rx_ctrl;
    uint8_t data[DATA_LENGTH];
    uint16_t cnt;
    uint16_t len;
};

static void printDataSpan(uint16_t size, uint8_t* data) {
  for(uint16_t i = 0; i < DATA_LENGTH && i < size; i++) {
    // Serial.write(data[i]);
  }
}

static void getMAC(char *addr, uint8_t* data) {
  sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[0], data[1], data[2], data[3], data[4], data[5]);
}

static void showMetadata(SnifferPacket *snifferPacket) {
  unsigned int frameControl = ((unsigned int)snifferPacket->data[1] << 8) + snifferPacket->data[0];

  uint8_t version      = (frameControl & 0b0000000000000011) >> 0;
  uint8_t frameType    = (frameControl & 0b0000000000001100) >> 2;
  uint8_t frameSubType = (frameControl & 0b0000000011110000) >> 4;
  uint8_t toDS         = (frameControl & 0b0000000100000000) >> 8;
  uint8_t fromDS       = (frameControl & 0b0000001000000000) >> 9;

  // Only look for probe request packets
  if (frameType != TYPE_MANAGEMENT ||
      frameSubType != SUBTYPE_PROBE_REQUEST)
        return;

  // Serial.print("RSSI: ");
  // Serial.print(snifferPacket->rx_ctrl.rssi, DEC);

  // Serial.print(" Ch: ");
  // Serial.print(wifi_get_channel());

  char addr[] = "00:00:00:00:00:00";
  getMAC(addr, snifferPacket->data + 10);
  // Serial.print(" Source MAC: ");
  // Serial.print(addr);

  int16_t idx = find_in_mac_list(snifferPacket->data + 10); // is mac in the list
  if (idx == -1){ // NO
    add_to_mac_list(snifferPacket->data + 10); // add it
    // Serial.print(" NEW ");
  }
  else { // YES
    update_timestamp_in_mac_list(idx); // update it
    // Serial.print(" OLD ");
  }

  /* getMAC(addr, snifferPacket->data, 4);
  // Serial.print(" Destination MAC: ");
  // Serial.print(addr); */ // this is always FF:FF:FF:...

  uint8_t SSID_length = snifferPacket->data[25];
  if (SSID_length > 0){
    // Serial.print(" SSID: ");
    // printDataSpan(SSID_length, snifferPacket->data + 26);
  }

  uint8_t mac0 = snifferPacket->data[10];
  // if (((mac0 | 0x02) & 0xfe) == mac0) Serial.print(" Locally administered! ");

  // Serial.println();
}

/**
 * Callback for promiscuous mode
 */
static void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length) {
  struct SnifferPacket *snifferPacket = (struct SnifferPacket*) buffer;
  showMetadata(snifferPacket);
}

#define CHANNEL_HOP_INTERVAL_MS   1000
void channelHop()
{
  // hoping channels 1-13 // 14 is not available in ESP8266-01
  uint8 new_channel = wifi_get_channel() + 1;
  if (new_channel > 13)
    new_channel = 1;
  wifi_set_channel(new_channel);
}

#define DISABLE 0
#define ENABLE  1

void setup() {
  for (uint16_t i = 0; i < size_limit_of_mac_list; i++)
    mac_list[i].timestamp = 0; // clear all items
  
  Serial.begin(115200);
  Serial.println();

  // setup LoRa transceiver module
  Serial.print("Initializing LoRa ");
  LoRa.setPins(SS, RST, DIO0);
  while (!LoRa.begin(433E6)) { // 433E6, 866E6, 915E6
    Serial.print(". ");
    delay(500);
  }
  LoRa.setSyncWord(0xA2);
  LoRa.setTxPower(20); // 2 - 20
  LoRa.setSpreadingFactor(LoraMode<LORA_MODE>::spreadingfactor);
  LoRa.setSignalBandwidth(LoraMode<LORA_MODE>::bandwidth);
  LoRa.setCodingRate4(LoraMode<LORA_MODE>::codingrate);
  LoRa.enableCrc();
  Serial.println("OK!");

  // setup WiFi Monitor Mode
  Serial.print("Initializing WiFi Monitor Mode ");
  delay(10);
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(1);
  wifi_promiscuous_enable(DISABLE);
  delay(10);
  wifi_set_promiscuous_rx_cb(sniffer_callback);
  delay(10);
  wifi_promiscuous_enable(ENABLE);
  Serial.println("OK!");
}

uint32_t last_clean_second = 0;
void loop() {
  delay(CHANNEL_HOP_INTERVAL_MS);
  channelHop();
  uint32_t sec = get_seconds();
  
  if (sec - last_clean_second > 30){ // make a cleaning every 30 secs
    last_clean_second = sec;

    Serial.print("cnt before clean: ");
    Serial.println(count_of_macs);
    remove_more_than_5_mins(); // clean old macs
    Serial.print("cnt after clean: ");
    Serial.println(count_of_macs);

    LoRa.beginPacket();
    LoRa.print("caftra:"); // just a way to distinguish our messages
    LoRa.print(count_of_macs);
    LoRa.endPacket(true); // send it async, doesn't matter if fails since it will try again in 30 seconds
  }
}
