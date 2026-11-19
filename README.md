# LoRaWAN Signal Checker

Gather signal strength &amp; quality of LoRaWAN Network (Chirpstack) using ESP32.

## 1. Hardware
### Requirements
* ESP32 (e.g ESP32-C3 Super Mini)
* LoRa Module (e.g SX1262)
* Breadboard and Jumper Cables
> This project used ESP32-C3 and SX1262 (Heltec's HT-RA62), but you can use any ESP32/LoRa Module just to know the pinout.
### Pinout
| ESP32-C3      | HT-RA62 (SX1262) |
| :-----------: |:----------------:|
| 3V3           | 3V3              |
| GND           | GND              |
| 7             | NSS              |
| 6             | MOSI             |
| 5             | MISO             |
| 4             | SCK              |
| 3             | DIO1             |
| 1             | RESET            |
| 0             | BUSY             |
## 2. Software
### Configuration (main.ino)
#### Pin Mapping
```cpp
#define PIN_NSS   7
#define PIN_DIO1  3
#define PIN_NRST  1
#define PIN_BUSY  0
#define PIN_SCK   4
#define PIN_MISO  5
#define PIN_MOSI  6
```
#### Region/Frequency Selection
```cpp
LoRaWANNode node(phy, &EU868); // Change region if needed (e.g., &US915)
```
#### ChirpStack Credentials
```cpp
uint64_t devEUI = 0x0000000000000000ULL; // Device EUI
uint64_t joinEUI = 0x0000000000000000ULL; // Join EUI

uint8_t appKey[] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}; // App Key

uint8_t nwkKey[] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
}; // Sometimes it is same as App Key, depends on LoRaWAN Version
```
#### Uplink Interval
```cpp
const unsigned long UPLINK_INTERVAL = 20000;
```
#### Example Data Logs
```
======================================
 LoRaWAN Signal Checker
======================================
[RadioLib] Initializing LoRa Module... OK
[NVS] Restored previous LoRaWAN session from flash!
[LoRaWAN] Activating LoRaWAN session... SESSION RESTORED FROM NVS!
[NVS] LoRaWAN session & nonces saved successfully.
--------------------------------------
[Uplink #1] Sending payload: "Test Packet #1"
[Uplink] Sent successfully (No downlink received).
[NVS] LoRaWAN session & nonces saved successfully.
[Signal] Last Downlink RSSI: -39.00 dBm | SNR: -18.00 dB
--------------------------------------
```
> Those logs come into **Serial Monitor** at the baud rate of 115200.
## 3. Ending
### Bugs
* To be seen
### Improvements
* Integrate Deep Sleep
* Integrate Displaying on a 0.96 Screen
