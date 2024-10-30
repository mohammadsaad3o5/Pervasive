#include <Arduino.h>
#include <SPI.h>
#include "Arducam_Mega.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "base64.h"
#include <ArduinoJson.h>
#include <algorithm>
#include "Talkie.h"
#include "ColourVocab.h"
#include "phraseVocab.h"

// Arducam configuration
#define CS_PIN 14
#define SPEAKER_PIN D2
#define BUTTON_PIN D3


Arducam_Mega myCAM(CS_PIN);



// WiFi credentials
const char* ssid = "GalaxyS23"; 
const char* password = "saadbhai";

// API endpoints
const char* detectColourURL = "https://ispy-api-production.up.railway.app/process-image/";
const char* compareImageURL = "https://ispy-api-production.up.railway.app/compare-image";

// Global variable to hold the color name
String colorName;

// Variables to hold image data
uint8_t* firstImageBuffer = nullptr;
uint32_t firstImageSize = 0;

// Timing variables
unsigned long previousMillis = 0;
const unsigned long interval = 10000; // 10 seconds delay between captures

Talkie voice;

void sayColour(String colour) {
    Serial.println(colour);
    if(colour.equalsIgnoreCase("red")) {
        voice.say(red);
    }
    else if(colour.equalsIgnoreCase("black")) {
        voice.say(black);
    }
    else if(colour.equalsIgnoreCase("blue")) {
        voice.say(blue);
    }
    else if(colour.equalsIgnoreCase("green")) {
        voice.say(green);
    }
    else if(colour.equalsIgnoreCase("white")) {
        voice.say(white);
    }
    else if(colour.equalsIgnoreCase("yellow")) {
        voice.say(yellow);
    }
    else if(colour.equalsIgnoreCase("gray")) {
        voice.say(gray);
    }
    else if(colour.equalsIgnoreCase("magenta")) {
        voice.say(magenta);
    }
    else if(colour.equalsIgnoreCase("cyan")) {
        voice.say(cyan);
    }
    else if(colour.equalsIgnoreCase("beige")) {
        voice.say(beige);
    }
    else if(colour.equalsIgnoreCase("teal")) {
        voice.say(teal);
    }
    else if(colour.equalsIgnoreCase("oliveGreen")) {
        voice.say(oliveGreen);
    }
    else if(colour.equalsIgnoreCase("navyBlue")) {
        voice.say(navyBlue);
    }
    else if(colour.equalsIgnoreCase("maroon")) {
        voice.say(maroon);
    }
    else if(colour.equalsIgnoreCase("limeGreen")) {
        voice.say(limeGreen);
    }
    else if(colour.equalsIgnoreCase("turquoise")) {
        voice.say(turquoise);
    }
    else if(colour.equalsIgnoreCase("brown")) {
        voice.say(brown);
    }
    else if(colour.equalsIgnoreCase("pink")) {
        voice.say(pink);
    }
    else if(colour.equalsIgnoreCase("purple")) {
        voice.say(purple);
    }
    else if(colour.equalsIgnoreCase("orange")) {
        voice.say(orange);
    }
    else {
        voice.say(black);
    }
}

void colourMatchSuccess(String colour) {
    Serial.println("Colors match!");
    voice.say(successMessage);
    sayColour(colour);
}

void colourMatchFail(String colour) {
    Serial.println("Colors do not match!");
    voice.say(failMessage);
    sayColour(colour);
}

void swapDevices() {
    voice.say(passDevice);
}

void sayTakePicture() {
    voice.say(pleaseTakePicture);
}

void pictureTakenSuccess(String colour) {
    voice.say(picTakenOf);
    sayColour(colour);
}

void setup() {
  Serial.begin(250000);
  pinMode(SPEAKER_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  
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

String urlencode(String str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encodedString += '%';
            encodedString += '2';
            encodedString += '0';
        } else if (isalnum(c)) {
            encodedString += c;
        } else {
            code1 = (c & 0xF) + '0';
            if ((c & 0xF) > 9) {
                code1 = (c & 0xF) - 10 + 'A';
            }
            c = (c >> 4) & 0xF;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
    }
    return encodedString;
}

bool compareImageWithColor(const char* url, String colorName, uint8_t* imageBuffer, uint32_t imgLen, String& resultStr) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Construct the full URL with the color as a path parameter
        String fullUrl = String(url) + "/" + urlencode(colorName);

        Serial.print("Full URL: ");
        Serial.println(fullUrl);

        // Begin connection to server
        http.begin(fullUrl);
        http.addHeader("Content-Type", "application/octet-stream");

        // Send POST request with image data as body
        int httpResponseCode = http.POST(imageBuffer, imgLen);

        Serial.printf("HTTP Response code: %d\n", httpResponseCode);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.print("Response: ");
            Serial.println(response);

            // Parse JSON response to get result string
            DynamicJsonDocument doc(1024); // Adjust size as needed
            DeserializationError error = deserializeJson(doc, response);

            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                http.end();
                return false;
            }

            // Extract the "result" string from the JSON response
            resultStr = doc["result"].as<String>();
            Serial.print("Result from API: ");
            Serial.println(resultStr);

            http.end();
            return true;

        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
            http.end();
            return false;
        }
    } else {
        Serial.println("WiFi Disconnected");
        return false;
    }
}

int buttonState = 0;
int state = 0;

bool pleaseTakePictureAsked = false;
bool picTakenOfAsked = false;
bool sayFirstImageColourAsked = false;
bool passDeviceAsked = false;
bool statusSaid = false;
bool colourIndicatorAsked = false;

String extractSecondColor(const String& apiResult) {
    // Split the apiResult string by commas
    int firstComma = apiResult.indexOf(',');
    int secondComma = apiResult.indexOf(',', firstComma + 1);

    if (firstComma == -1 || secondComma == -1) {
        Serial.println("Error parsing API result.");
        return "";
    }

    // Extract the second color (colour Y)
    String secondColor = apiResult.substring(secondComma + 1);
    secondColor.trim(); // Remove any leading/trailing whitespace

    // Remove 'colour ' prefix if it exists
    String prefix = "colour ";
    if (secondColor.startsWith(prefix)) {
        secondColor = secondColor.substring(prefix.length());
    }

    Serial.print("Extracted second color: ");
    Serial.println(secondColor);

    return secondColor;
}

void loop() {
  static bool firstCaptureDone = false;
  static unsigned long firstCaptureTime = 0;

  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Disconnected");
      delay(1000);
      return;
  }

  if (!pleaseTakePictureAsked) {
    voice.say(pleaseTakePicture);
    pleaseTakePictureAsked = true;
  }

  // unsigned long currentMillis = millis();

  if (digitalRead(BUTTON_PIN) == HIGH) {
    state += 1;
  }


  if (state % 3 == 1 && !firstCaptureDone) {
    // Capture the first image
    if (captureImage(&firstImageBuffer, &firstImageSize)) {
      Serial.println("First image captured and stored.");

      // Send the first image to detect its colour
      if (sendImageAndGetColor(detectColourURL, firstImageBuffer, firstImageSize, colorName)) {
        Serial.println("Color name received and stored.");

        if (!picTakenOfAsked) {
          voice.say(picTakenOf);
          picTakenOfAsked = true;
        }

        if (!sayFirstImageColourAsked) {
          sayColour(colorName);
          sayFirstImageColourAsked = true;
        }

        if (!passDeviceAsked) {
          voice.say(passDevice);
          passDeviceAsked = true;
        }

        delay(5000); // Delay 5 seconds here

        if (!colourIndicatorAsked) {
          voice.say(colourIndicator);
          sayColour(colorName);
          colourIndicatorAsked = true;
        }

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
    // firstCaptureTime = currentMillis;
  } 
  else if (state % 3 == 2 && firstCaptureDone) {
    // Capture the second image
    uint8_t* secondImageBuffer = nullptr;
    uint32_t secondImageSize = 0;

    


    if (captureImage(&secondImageBuffer, &secondImageSize)) {
      Serial.println("Second image captured.");


      String apiResult;

      // Send the color name and second image to compare
      if (compareImageWithColor(compareImageURL, colorName, secondImageBuffer, secondImageSize, apiResult)) {
        // Successfully got result from API, proceed with logic based on apiResult
        Serial.print("API Result: ");
        Serial.println(apiResult);

        String secondColor = extractSecondColor(apiResult);

        // Build logic based on the apiResult string
        if (apiResult.startsWith("SAME")) {
          // The colors are exactly the same
          colourMatchSuccess(secondColor);
        } else if (apiResult.startsWith("CLOSE")) {
          // The colors are nearly the same
          colourMatchSuccess(secondColor);
        } else if (apiResult.startsWith("DIFFERENT")) {
          // The colors are different
          colourMatchFail(secondColor);
        } else {
          // Handle unexpected result
          Serial.println("Unexpected API result.");
        }

      } else {
        Serial.println("Failed to compare image with color.");
        // Handle error
      }


      // Clean up
      free(firstImageBuffer);
      firstImageBuffer = nullptr;
      firstImageSize = 0;

      free(secondImageBuffer);
      secondImageBuffer = nullptr;

      // Reset for next cycle
      firstCaptureDone = false;
      colorName = ""; // Clear the color name

      pleaseTakePictureAsked = false;
      picTakenOfAsked = false;
      sayFirstImageColourAsked = false;
      passDeviceAsked = false;
    } else {
      Serial.println("Second image capture failed.");
      // Decide whether to retry or reset
    }
  }

  // Small delay to prevent high CPU usage
  delay(150);
}
