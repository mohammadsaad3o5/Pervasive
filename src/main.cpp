// #include <Arduino.h>
// #include <SPI.h>
// #include "Arducam_Mega.h"
// #include <WiFi.h>
// #include <HTTPClient.h>

// // Arducam configuration
// #define CS_PIN 14
// Arducam_Mega myCAM(CS_PIN);

// // WiFi credentials
// const char* ssid = "iPhone"; 
// const char* password = "khongcopass";

// // API endpoint
// const char* serverName = "https://ispy-api-production.up.railway.app/process-image/";
// // const char* serverName = "http://localhost:8000/process-image/";

// void setup() {
//   Serial.begin(250000);
  
//   // Initialize WiFi
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//       delay(500);
//       Serial.print(".");
//   }
//   Serial.println("\nConnected to WiFi");

//   // Initialize ArduCAM
//   if (myCAM.begin() != 0) {
//       Serial.println("Failed to initialize ArduCAM Mega!");
//       while (1);
//   }
//   Serial.println("ArduCAM Mega initialized successfully!");
// }

// void sendImageToAPI(uint8_t* imageBuffer, uint32_t imgLen) {
//   // Send to API
//   HTTPClient http;
  
//   // Begin connection to server
//   http.begin(serverName);
  
//   // Set headers
//   http.addHeader("Content-Type", "application/octet-stream");
  
//   // Send POST request with image data
//   int httpResponseCode = http.POST(imageBuffer, imgLen);
  
//   // Handle response
//   if (httpResponseCode > 0) {
//       String response = http.getString();
//       Serial.printf("HTTP Response code: %d\n", httpResponseCode);
//       Serial.println("Response: " + response);
//   } else {
//       Serial.printf("Error code: %d\n", httpResponseCode);
//   }

//   // Clean up
//   http.end();
// }

// void captureAndSendImage() {
//   if (myCAM.takePicture(CAM_IMAGE_MODE_SVGA, CAM_IMAGE_PIX_FMT_JPG) != 0) {
//       Serial.println("Failed to capture image!");
//       return;
//   }
//   Serial.println("Image captured!");

//   // Get image size
//   uint32_t imgLen = myCAM.getTotalLength();
//   Serial.printf("Image size: %u bytes\n", imgLen);

//   // Allocate buffer for the complete image
//   uint8_t* imageBuffer = (uint8_t*)malloc(imgLen);
//   if (!imageBuffer) {
//       Serial.println("Failed to allocate memory!");
//       return;
//   }

//   // Read the complete image into buffer
//   size_t index = 0;
//   while (myCAM.getReceivedLength()) {
//       imageBuffer[index++] = myCAM.readByte();
//   }

//   // Send the image buffer to the API
//   sendImageToAPI(imageBuffer, imgLen);

//   // Clean up
//   free(imageBuffer);
// }

// void loop() {
//   if (WiFi.status() != WL_CONNECTED) {
//       Serial.println("WiFi Disconnected");
//       delay(1000);
//       return;
//   }

//   captureAndSendImage();

//   delay(60000);
// }


#include <Arduino.h>
#include <SPI.h>
#include "Arducam_Mega.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "base64.h"
#include <ArduinoJson.h>
#include <algorithm>

// Arducam configuration
#define CS_PIN 14
Arducam_Mega myCAM(CS_PIN);

// WiFi credentials
const char* ssid = "iPhone"; 
const char* password = "khongcopass";

// API endpoints
const char* detectColourURL = "https://ispy-api-production.up.railway.app/process-image/";
const char* compareImageURL = "https://ispy-api-production.up.railway.app/compare-image/";

// Global variable to hold the color name
String colorName;

// Variables to hold image data
uint8_t* firstImageBuffer = nullptr;
uint32_t firstImageSize = 0;

// Timing variables
unsigned long previousMillis = 0;
const unsigned long interval = 10000; // 10 seconds delay between captures

void setup() {
  Serial.begin(250000);
  pinMode(D3, INPUT);
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
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

const uint32_t TIMEOUT = 10000; // 10 seconds

bool captureImage(uint8_t** imageBuffer, uint32_t* imgLen) {

    if (myCAM.takePicture(CAM_IMAGE_MODE_SVGA, CAM_IMAGE_PIX_FMT_JPG) != 0) {
        Serial.println("Failed to capture image!");
        return false;
    }
    Serial.println("Image captured!");

    // Get image size
    *imgLen = myCAM.getTotalLength();
    Serial.printf("Image size: %u bytes\n", *imgLen);

    // Allocate buffer for the complete image
    *imageBuffer = (uint8_t*)malloc(*imgLen);
    if (!*imageBuffer) {
        Serial.println("Failed to allocate memory!");
        return false;
    }

    // Read the complete image into buffer
    size_t index = 0;
    while (myCAM.getReceivedLength()) {
        (*imageBuffer)[index++] = myCAM.readByte();
    }

    // Clear the FIFO buffer after reading
    // myCAM.clearFIFOBuffer();

    Serial.println("Image captured successfully!");
    return true;
}

bool sendImageAndGetColor(const char* url, uint8_t* imageBuffer, uint32_t imgLen, String& colorName) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;

    // Begin connection to server
    http.begin(url);

    // Set headers
    http.addHeader("Content-Type", "application/octet-stream");

    // Send POST request with image data
    int httpResponseCode = http.POST(imageBuffer, imgLen);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);

      // Parse JSON response to get color name
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        http.end();
        return false;
      }

      colorName = doc["color_name"].as<String>();
      Serial.print("Received Color Name: ");
      Serial.println(colorName);

    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
      http.end();
      return false;
    }

    // Free resources
    http.end();
    return true;
  } else {
    Serial.println("WiFi Disconnected");
    return false;
  }
}

void compareImageWithColor(const char* url, String colorName, uint8_t* imageBuffer, uint32_t imgLen) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;

    // Begin connection to server
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Encode image to base64
    String image_base64 = base64::encode(imageBuffer, imgLen);

    // Create JSON payload
    const size_t capacity = JSON_OBJECT_SIZE(2) + colorName.length() + image_base64.length();
    DynamicJsonDocument doc(capacity);

    doc["color"] = colorName;
    doc["image"] = image_base64;

    String payload;
    serializeJson(doc, payload);

    // Send POST request
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);

      // Parse JSON response to get comparison result
      DynamicJsonDocument respDoc(1024);
      DeserializationError error = deserializeJson(respDoc, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      } else {
        String result = respDoc["result"].as<String>();
        Serial.print("Comparison Result: ");
        Serial.println(result);
      }
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

int buttonState = 0;
int state = 0;
void loop() {
  static bool firstCaptureDone = false;
  static unsigned long firstCaptureTime = 0;

  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Disconnected");
      delay(1000);
      return;
  }

  // buttonState = digitalRead(D3);
  Serial.println(digitalRead(D3));

  unsigned long currentMillis = millis();

  if (digitalRead(D3) == HIGH) {
    state += 1;
  }


  if (state % 3 == 1 && !firstCaptureDone) {
    // Capture the first image
    if (captureImage(&firstImageBuffer, &firstImageSize)) {
      Serial.println("First image captured and stored.");

      // Send the first image to detect its colour
      if (sendImageAndGetColor(detectColourURL, firstImageBuffer, firstImageSize, colorName)) {
        Serial.println("Color name received and stored.");
      } else {
        Serial.println("Failed to get color name.");
        // Clean up
        free(firstImageBuffer);
        firstImageBuffer = nullptr;
        firstImageSize = 0;
        delay(1000); // Retry after delay
        return;
      }
    } else {
      Serial.println("First image capture failed.");
      delay(1000); // Retry after delay
      return;
    }
    firstCaptureDone = true;
    firstCaptureTime = currentMillis;
  } 
  else if (state % 3 == 2 && firstCaptureDone) {
    // Capture the second image
    uint8_t* secondImageBuffer = nullptr;
    uint32_t secondImageSize = 0;
    if (captureImage(&secondImageBuffer, &secondImageSize)) {
      Serial.println("Second image captured.");

      // Send the color name and second image to compare
      compareImageWithColor(compareImageURL, colorName, secondImageBuffer, secondImageSize);

      // Clean up
      free(firstImageBuffer);
      firstImageBuffer = nullptr;
      firstImageSize = 0;

      free(secondImageBuffer);
      secondImageBuffer = nullptr;

      // Reset for next cycle
      firstCaptureDone = false;
      colorName = ""; // Clear the color name
    } else {
      Serial.println("Second image capture failed.");
      // Decide whether to retry or reset
    }
  }

  // Small delay to prevent high CPU usage
  delay(150);
}
