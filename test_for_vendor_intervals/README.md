## PoC for probe sniffing with ESP8266 and Testing probe intervals of various devices

In this part of the project, we wrote a small firmware for ESP8266 that sniffs all WiFi traffic (changes channel every 1 second; however, cannot scan channel 14 since it is not supported by ESP8266-01), filters the probe requests and prints the relevant data to PC through Serial Port.

In addition to that, we wrote a small python script that parses the MAC-Vendor list from Wireshark that can be found [here](https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob_plain;f=manuf). After that, this script reads the serial packets that are coming from ESP and finds the manufacturer of the captured packets, scipt can also detect Locally Administered MAC Addresses. If there is another request that came from this device in the last 5 seconds, the script drops the packet. If the second request came more than 5 seconds later, then we save the time difference to process later.

Finally the script prints 10 most received mac address' manufacturer name + time differences between packets.

## Approximate probe intervals per vendor

Vendor | Interval
:---: | :---:
Apple | 45 sec
Intel | 60 sec
HTC | 15 sec
Samsung | 25 sec
Google | 60 sec

## Conclusion

Since no vendor seems like using more than 1 minute as interval, we can keep the mac address window as ~5 minutes (we may miss some packets) and after 5 minutes we may assume the device has left the place.