# CantinTraffic

The aim of this project is to approximately measure the crowdedness of the Bogazici University North Campus Dining Hall.

We will use ESP8266 for capturing probe requests and by using a 5 minute window, we will count the unique MAC addresses in these probe requests. By using LoRa, we will send these information to a relay and this relay will send the data to cloud and finally the cloud will serve the data in a web server.

## PoC - Vendor Intervals

The first deliverable of the project is a test for Vendor specific Probe request interval tester. You can find information about this test [here](test_for_vendor_intervals).