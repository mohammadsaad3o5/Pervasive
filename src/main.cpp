#include <Arduino.h>
#include <SPI.h>
#include "Arducam_Mega.h"
#include <WiFi.h>
#include <HTTPClient.h>
// #include <ArduinoJson.h>


void printBuffer(uint8_t *buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        // Print each byte in hexadecimal format
        printf("0x%02X ", buffer[i]);

        // Optional: Add a new line every 16 bytes for better readability
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");  // Newline after printing the buffer
}



// Initialize the HTTP client to send requests to the Gemini API.
HTTPClient http_client;
WiFiServer wifi_server(80);
// WiFi credentials and device name.
const char* ssid = "GalaxyS23"; 
const char* password = "saadbhai";
const char* device_name = "saad"; 
// Arducam
#define CS_PIN 14
Arducam_Mega myCAM(CS_PIN);
uint8_t imageData = 0;
uint8_t imageDataNext = 0;
uint8_t headFlag = 0;
const size_t bufferSize = 2048;
uint8_t buffer[bufferSize] = {0};
// Server stuff
String serverIP = "";
uint8_t serverPort = 80;


// #include <Arduino.h>
// #include <SPI.h>
// #include "Arducam_Mega.h"

// #define CS_PIN 14


// Arducam_Mega myCAM(CS_PIN);

void setup() {
  Serial.begin(250000);
  SPI.begin(18, 19, 23, CS_PIN);

  // Serial.println("Initializing saad's ArduCAM Mega...");

  if (myCAM.begin() != 0) {
    // Serial.println("Failed to initialize ArduCAM Mega!");
    while (1);
  }
  Serial.println(myCAM.begin());
  // Serial.println("ArduCAM Mega initialized successfully ALHAMDULLILAH!");

}

void loop() {
  // Capture image
  // Serial.println("Capturing image...");
  if (myCAM.takePicture(CAM_IMAGE_MODE_SVGA,CAM_IMAGE_PIX_FMT_JPG) != 0) {
    Serial.println("Failed to capture image!");
    return;
  }
  // Serial.println("Image captured!");

  // Get the size of the image
  uint32_t imgLen = myCAM.getTotalLength();
  // Serial.print("Image length: ");
  Serial.println(imgLen);
  uint32_t receiveLen = myCAM.getReceivedLength();
  // Serial.print("Received length: ");
  // Serial.println(receiveLen);
  uint8_t imageData, imageDataNext;

  uint8_t buffer[256];  // Adjust buffer size as needed
  uint16_t i = 0;
  uint8_t headFlag = 0;  // Flag to detect JPEG header

  while (myCAM.getReceivedLength()) {
      // Shift the data for the next comparison
      imageData = imageDataNext;
      imageDataNext = myCAM.readByte();

      // If the JPEG header has been found
      if (headFlag == 1) {
          buffer[i++] = imageDataNext;  // Store the byte in the buffer

          // If the buffer is full, write it to the Serial
          if (i >= sizeof(buffer)) {
              Serial.write(buffer, i);  // Send raw binary data
              i = 0;  // Reset the buffer index
          }
      }

      // Detect JPEG start marker (0xFFD8)
      if (imageData == 0xFF && imageDataNext == 0xD8) {
          headFlag = 1;  // Start storing image data
          // Serial.println("Start");
          buffer[i++] = imageData;     // Add JPEG start marker to the buffer
          buffer[i++] = imageDataNext;
      }

      // Detect JPEG end marker (0xFFD9)
      if (imageData == 0xFF && imageDataNext == 0xD9) {
          headFlag = 0;  // Stop storing image data
          Serial.write(buffer, i);  // Send any remaining data
          i = 0;  // Reset the buffer index
          // Serial.println("Finished");
          break;  // Exit the loop after the JPEG end marker is found
      }
  }

  delay(500000); // 5 seconds
  exit(0); 
}






