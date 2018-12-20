# CantinTraffic

The aim of this project is to approximately measure the crowdedness of the Bogazici University North Campus Dining Hall.

We will use ESP8266 for capturing probe requests and by using a 5 minute window, we will count the unique MAC addresses in these probe requests. By using LoRa, we will send these information to a relay and this relay will send the data to cloud and finally the cloud will serve the data in a web server.

---
---
---

## PoC - Vendor Intervals

The first deliverable of the project is a test for Vendor specific Probe request interval tester. You can find information about this test [here](test_for_vendor_intervals).

---
---
---

## Final Project
### Sender (Collect)

The first part of the project is an ESP-12e project (also works with ESP-32 with pin change). I used ESP-12e instead of ESP8266 because SX1278 requires at least 5 IO pins and ESP8266 has only 3. You can find the code [here](esp_send_project).

This program receives the probe requests, keeps track of the mac addresses in 5 minute window and sends the count of unique mac addresses using LoRa module SX1278. It doesn't require anything to work except power since it passively listens the WiFi packets and the LoRa is connectionless like UDP so it can just send a message without anyone listening.

---

### Receiver (Relay)

This part of the project is an ESP-32 project (also works with ESP-12e with pin change). I used ESP-32 instead of ESP8266 because SX1278 requires at least 5 IO pins and ESP8266 has only 3. You can find the code [here](esp_recv_project).

This program listens for LoRa messages and also it maintains a connection with a WiFi AP to send the data from LoRa to cloud. Whenever it receives a LoRa message, it sends a GET request to cloud and cloud logs it with time. It requires a WiFi AP in order to work.

---

### Cloud (Store)

Cloud receives the data from Receiver and stores it with timestamps. You can find the code [here](server_codes).