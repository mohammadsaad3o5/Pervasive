#include <Arduino.h>
#include <SPI.h>
#include "Arducam_Mega.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Arducam configuration
#define CS_PIN 14
Arducam_Mega myCAM(CS_PIN);

// WiFi credentials
const char* ssid = "iPhone"; 
const char* password = "khongcopass";

// API endpoint
const char* serverName = "https://ispy-api-production.up.railway.app/process-image/";
// const char* serverName = "http://localhost:8000/process-image/";

void setup() {
  Serial.begin(250000);
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize ArduCAM
  if (myCAM.begin() != 0) {
      Serial.println("Failed to initialize ArduCAM Mega!");
      while (1);
  }
  Serial.println("ArduCAM Mega initialized successfully!");
}

void sendImageToAPI(uint8_t* imageBuffer, uint32_t imgLen) {
  // Send to API
  HTTPClient http;
  
  // Begin connection to server
  http.begin(serverName);
  
  // Set headers
  http.addHeader("Content-Type", "application/octet-stream");
  
  // Send POST request with image data
  int httpResponseCode = http.POST(imageBuffer, imgLen);
  
  // Handle response
  if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      Serial.println("Response: " + response);
  } else {
      Serial.printf("Error code: %d\n", httpResponseCode);
  }

  // Clean up
  http.end();
}

void captureAndSendImage() {
  if (myCAM.takePicture(CAM_IMAGE_MODE_SVGA, CAM_IMAGE_PIX_FMT_JPG) != 0) {
      Serial.println("Failed to capture image!");
      return;
  }
  Serial.println("Image captured!");

  // Get image size
  uint32_t imgLen = myCAM.getTotalLength();
  Serial.printf("Image size: %u bytes\n", imgLen);

  // Allocate buffer for the complete image
  uint8_t* imageBuffer = (uint8_t*)malloc(imgLen);
  if (!imageBuffer) {
      Serial.println("Failed to allocate memory!");
      return;
  }

  // Read the complete image into buffer
  size_t index = 0;
  while (myCAM.getReceivedLength()) {
      imageBuffer[index++] = myCAM.readByte();
  }

  // Send the image buffer to the API
  sendImageToAPI(imageBuffer, imgLen);

  // Clean up
  free(imageBuffer);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Disconnected");
      delay(1000);
      return;
  }

  captureAndSendImage();
}




