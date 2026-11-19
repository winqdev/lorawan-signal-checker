#include <RadioLib.h>
#include <Preferences.h>

// --- PIN DEFINITIONS FOR ESP32-C3 SuperMini + HT-RA62 (SX1262) ---
#define PIN_NSS   7
#define PIN_DIO1  3
#define PIN_NRST  1
#define PIN_BUSY  0
#define PIN_SCK   4
#define PIN_MISO  5
#define PIN_MOSI  6

// Create radio instance
SX1262 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);

// Create LoRaWAN node instance
PhysicalLayer* phy = &radio;
LoRaWANNode node(phy, &EU868); // Change region if needed (e.g., &US915)

// --- CHIRPSTACK CREDENTIALS ---
uint64_t devEUI = 0xeb3b73688faf6ba2ULL;
uint64_t joinEUI = 0xb62c46c2c0fe82e3ULL;

uint8_t appKey[] = { 
  0x3e, 0x4b, 0x80, 0x4e, 0xc0, 0xe5, 0x3c, 0x34, 
  0xe7, 0x68, 0xdd, 0x74, 0xa7, 0x4d, 0x51, 0xb9 
};

uint8_t nwkKey[] = { 
  0x3e, 0x4b, 0x80, 0x4e, 0xc0, 0xe5, 0x3c, 0x34, 
  0xe7, 0x68, 0xdd, 0x74, 0xa7, 0x4d, 0x51, 0xb9 
};

// --- NVS PREFERENCES SETUP ---
Preferences preferences;
const char* nvsNamespace = "w";

// Interval between uplink transmissions (in milliseconds)
const unsigned long UPLINK_INTERVAL = 20000;
unsigned long lastUplinkTime = 0;
uint32_t packetCounter = 0;

void saveSessionToNVS() {
  preferences.begin(nvsNamespace, false);
  
  uint8_t sessionBuf[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
  memcpy(sessionBuf, node.getBufferSession(), RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
  preferences.putBytes("session", sessionBuf, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

  uint8_t noncesBuf[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
  memcpy(noncesBuf, node.getBufferNonces(), RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
  preferences.putBytes("nonces", noncesBuf, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);

  preferences.end();
  Serial.println(F("[NVS] LoRaWAN session & nonces saved successfully."));
}

bool loadSessionFromNVS() {
  preferences.begin(nvsNamespace, true);
  
  size_t sessionLen = preferences.getBytesLength("session");
  size_t noncesLen = preferences.getBytesLength("nonces");

  if (sessionLen == RADIOLIB_LORAWAN_SESSION_BUF_SIZE && noncesLen == RADIOLIB_LORAWAN_NONCES_BUF_SIZE) {
    uint8_t sessionBuf[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
    uint8_t noncesBuf[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];

    preferences.getBytes("session", sessionBuf, sessionLen);
    preferences.getBytes("nonces", noncesBuf, noncesLen);
    preferences.end();

    int stateNonces = node.setBufferNonces(noncesBuf);
    int stateSession = node.setBufferSession(sessionBuf);

    if (stateNonces == RADIOLIB_ERR_NONE && stateSession == RADIOLIB_ERR_NONE) {
      Serial.println(F("[NVS] Restored previous LoRaWAN session from flash!"));
      return true;
    }
  } else {
    preferences.end();
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("\n======================================"));
  Serial.println(F(" LoRaWAN Signal Checker  "));
  Serial.println(F("======================================"));

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_NSS);

  // Initialize radio
  Serial.print(F("[RadioLib] Initializing LoRa Module... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("OK"));
  } else {
    Serial.print(F("Failed, code "));
    Serial.println(state);
    while (true);
  }

  // Configure LoRaWAN credentials
  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);

  // Restore nonces/session if available, then activate LoRaWAN
  loadSessionFromNVS();

  Serial.print(F("[LoRaWAN] Activating LoRaWAN session... "));
  state = node.activateOTAA();

  if (state == RADIOLIB_LORAWAN_NEW_SESSION || state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
    Serial.println(state == RADIOLIB_LORAWAN_NEW_SESSION
      ? F("JOINED NEW SESSION!")
      : F("SESSION RESTORED FROM NVS!"));
    saveSessionToNVS();
  } else {
    Serial.print(F("Join failed, code "));
    Serial.println(state);
    while (true);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUplinkTime >= UPLINK_INTERVAL || lastUplinkTime == 0) {
    lastUplinkTime = currentMillis;
    packetCounter++;

    String payloadStr = "Test Packet #" + String(packetCounter);
    uint8_t payload[32];
    payloadStr.getBytes(payload, sizeof(payload));

    Serial.println(F("--------------------------------------"));
    Serial.print(F("[Uplink #"));
    Serial.print(packetCounter);
    Serial.print(F("] Sending payload: \""));
    Serial.print(payloadStr);
    Serial.println(F("\""));

    int state = node.sendReceive(payload, payloadStr.length(), 1);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("[Uplink] Sent successfully (No downlink received)."));
    } else if (state == RADIOLIB_LORAWAN_DOWNLINK) {
      Serial.println(F("[Uplink] Sent successfully AND received downlink data!"));
    } else {
      Serial.print(F("[Uplink] Error during transmission, code: "));
      Serial.println(state);
    }

    saveSessionToNVS();

    if (radio.getRSSI() != 0) {
      Serial.print(F("[Signal] Last Downlink RSSI: "));
      Serial.print(radio.getRSSI());
      Serial.print(F(" dBm | SNR: "));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));
    }
  }
}
