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

// String sendToServer(String input) {
//   // Check if the client can connect to the Gemini API.
//  if (!http_client.begin(server)) {
//   Serial.println("Connection to API server failed!");
//   return "";
//  }
//  // Create the request body for the Gemini API.
//  http_client.addHeader("Content-Type", "application/json");
//  String requestBody = "{\"contents\":[{\"parts\":[{\"text\":\"" + project_description + input + "\"}]}]}";
//   // Send the request to the Gemini API and check if the response is successful.
//   int httpCode = http_client.POST(requestBody);
//   if (httpCode != 200) {
//     Serial.println("Failed to send request to Gemini");
//     Serial.println(requestBody);
//     return "";
//   }
//  // Parse the response from the Gemini API and extract the advice.
//   // String response = http_client.getString();
//   // StaticJsonDocument<1024> jsonDoc;
//   // deserializeJson(jsonDoc, response);
//   // String advice = jsonDoc["candidates"][0]["content"]["parts"][0]["text"];
//   // return advice;
// }


// void setup() {
//   Serial.begin(250000);
//   SPI.begin(18, 19, 23, CS_PIN);  

//   Serial.println("Initializing ArduCAM Mega...");

//   if (myCAM.begin() != 0) {
//     Serial.println("Failed to initialize ArduCAM Mega!");
//     while (1);  // Halt if initialization fails
//   }
//   Serial.println("ArduCAM Mega initialized successfully!");
//   WiFi.setHostname(device_name);
//   WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting to Wi-Fi...");
//   }
//   Serial.println("Connected to Wi-Fi");
//   Serial.print("IP Address: ");
//   Serial.println(WiFi.localIP());
// }

// // // Function to establish a connection to the server
// // WiFiClient connectToServer() {
// //   WiFiClient client;
// //   if (client.connect(serverIP, serverPort)) {
// //     Serial.println("Successfully connected to the server.");
// //   } else {
// //     Serial.println("Connection to server failed.");
// //   }
// //   return client;
// // }


// void loop() {
//   // Capture image
//   Serial.println("Capturing image...");
//   if (myCAM.takePicture(CAM_IMAGE_MODE_HD, CAM_IMAGE_PIX_FMT_YUV) != 0) {
//     Serial.println("Failed to capture image!");
//     return;
//   }
//   Serial.println("Image captured!");

//   // Get the size of the image
//   uint32_t imgLen = myCAM.getTotalLength();
//   Serial.print("Image length: ");
//   Serial.println(imgLen);


//   // Read and send image data
//   const uint16_t bufferSize = 256;  // Define buffer size
//   uint8_t buffer[bufferSize];
//   uint32_t bytesToSend = imgLen;
//   int i = 0;
//   Serial.printf("Unread image %d\n", myCAM.getReceivedLength());

//   while (myCAM.getReceivedLength()) {
//     imageData = imageDataNext;
//     imageDataNext = myCAM.readByte();

//     if (headFlag == 1) {
//       buffer[i++]=imageDataNext;  
//       if (i >= bufferSize) {
//       Serial.println("Start");
//       // printBuffer(buffer, bufferSize);

//       // Serial.println("Finish");
//       // Serial.printf("Unread image %d\n", myCAM.getReceivedLength());
//       i = 0;
//       }
//     }
//     if (imageData == 0xff && imageDataNext ==0xd8)
//       {
//           headFlag = 1;
//           Serial.println("Start");
//           buffer[i++]=imageData;
//           buffer[i++]=imageDataNext;  
//       }
//     if (imageData == 0xff && imageDataNext ==0xd9)
//         {
//             headFlag = 0;
//             // printBuffer(buffer, bufferSize);
//             i = 0;
//             Serial.println("finish");
//             break;
//         }
//   }
    



//   // Send end marker
//   // Serial.write("END");  // 3 bytes: 'E', 'N', 'D'

//   Serial.println("Image data sent!");

//   // Optionally, you can stop further execution or reset the camera
//   delay(50000); // Wait 50 seconds before capturing the next image
// }





















// #include <Arduino.h>
// #include <SPI.h>
// #include "Arducam_Mega.h"

// #define CS_PIN 14


// Arducam_Mega myCAM(CS_PIN);

void setup() {
  Serial.begin(250000);
  SPI.begin(18, 19, 23, CS_PIN);

  
  uint8_t BUFFER[256];
  uint8_t bytesRead = myCAM.readBuff(BUFFER, 100);
  Serial.println("bytesRead");
  Serial.println(bytesRead);

  Serial.println("Initializing saad's ArduCAM Mega...");

  // if (!myCAM.begin()) {
    // Serial.println("Failed to initialize ArduCAM Mega!");
    // while (1);
  // }
  Serial.println(myCAM.begin());
  // Serial.println("ArduCAM Mega initialized successfully ALHAMDULLILAH!");

}

void loop() {
  // Capture image
  Serial.println("Capturing image...");
  if (myCAM.takePicture(CAM_IMAGE_MODE_WQXGA2,CAM_IMAGE_PIX_FMT_JPG) != 0) {
    Serial.println("Failed to capture image!");
    return;
  }
  Serial.println("Image captured!");

  // Get the size of the image
  uint32_t imgLen = myCAM.getTotalLength();
  Serial.print("Image length: ");
  Serial.println(imgLen);
  uint32_t receiveLen = myCAM.getReceivedLength();
  Serial.print("Received length: ");
  Serial.println(receiveLen);
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
              // Serial.write(buffer, i);  // Send raw binary data
              i = 0;  // Reset the buffer index
          }
      }

      // Detect JPEG start marker (0xFFD8)
      if (imageData == 0xFF && imageDataNext == 0xD8) {
          headFlag = 1;  // Start storing image data
          Serial.println("Start");
          buffer[i++] = imageData;     // Add JPEG start marker to the buffer
          buffer[i++] = imageDataNext;
      }

      // Detect JPEG end marker (0xFFD9)
      if (imageData == 0xFF && imageDataNext == 0xD9) {
          headFlag = 0;  // Stop storing image data
          Serial.write(buffer, i);  // Send any remaining data
          i = 0;  // Reset the buffer index
          Serial.println("Finished");
          break;  // Exit the loop after the JPEG end marker is found
      }
  }

  delay(500000); // 5 seconds
  exit(0); 
}




























// // Gemini API key and server URL.
// const char* api_key = "AIzaSyAUAWQZG4n8smIWuqj3bfe-iN9yw-aQ0eI"; // TODO: Replace with your Gemini API key
// String gemini_server = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + String(api_key);
// String project_description = "Answer in one word: randomly pick either 1, 2 or 3; nothing else, just send exactly one character as a response"; // TODO: Use this to tell the Gemini API about the project.
// // Temporary: Temperature descriptions to send to the Gemini API for testing.
// const char* temperature_descriptions[3] = {"It is too cold", "It is too hot", "It is just right"};
// int current_description = 0;
// String sendToGemini(String input) {
//   // Check if the client can connect to the Gemini API.
//  if (!http_client.begin(gemini_server)) {
//   Serial.println("Connection to API failed!");
//   return "";
//  }
//  // Create the request body for the Gemini API.
//  http_client.addHeader("Content-Type", "application/json");
//  String requestBody = "{\"contents\":[{\"parts\":[{\"text\":\"" + project_description + input + "\"}]}]}";
//   // Send the request to the Gemini API and check if the response is successful.
//   int httpCode = http_client.POST(requestBody);
//   if (httpCode != 200) {
//     Serial.println("Failed to send request to Gemini");
//     Serial.println(requestBody);
//     return "";
//   }
//  // Parse the response from the Gemini API and extract the advice.
//   String response = http_client.getString();
//   StaticJsonDocument<1024> jsonDoc;
//   deserializeJson(jsonDoc, response);
//   String advice = jsonDoc["candidates"][0]["content"]["parts"][0]["text"];
//   return advice;
// }
// void processResponse(String advice) {
//  // TODO: Process the advice from the Gemini API and take appropriate action.
//   // Turn off the fan if the advice is to increase the temperature.

//  // Turn on the fan if the advice is to decrease the temperature.
//  // Turn on the LED if the advice is to maintain the temperature.
// }
// void setup() {
//   pinMode(D3, OUTPUT); 
//  // Initialize the serial monitor.
//  Serial.begin(115200);
 // Connect to the Wi-Fi network.
  
 // Start the server to listen for requests from the MCU-2.
//  wifi_server.begin();
//  // Set wait time for HTTP requests.
//  http_client.setTimeout(15000);
// }

// void loop() {
//   for(int x = 0; x <= 255; x+=5){
//       analogWrite(D3, x);
//       delay(50);
//     }

//     for(int x = 255; x >= 0; x-=5){
//       analogWrite(D3, x);
//       delay(50);
//     }
//   if (WiFi.status() == WL_CONNECTED) { // Check if the Wi-Fi is connected.
//  String geminiResponse = sendToGemini(String(temperature_descriptions[current_description]));
// // Send the request to the Gemini API.
//  // Temporary: Print the response from the Gemini API.
//  Serial.print("Temperature Description: ");
//  Serial.println(temperature_descriptions[current_description]);
//  Serial.println("Advice: " + geminiResponse);
//  // TODO: Process the response from the Gemini API.
//  // processResponse(geminiResponse);
//  current_description = (current_description + 1) % 3; // Cycle through the temperature descriptions.
//  }
//   delay(5000); // Delay for 5 seconds as rate limit for Gemini API is 15 requests per minute.
//  // TODO: Comment the above code, read the request from the MCU-2 through WiFi and send it to Gemini API.
//  // WiFiClient client = wifi_server.available();
// //  if (client && client.connected() && client.available()) {
// //  String request = client.readStringUntil('\r');
// //  client.flush();
// //  String geminiResponse = sendToGemini(request);
// //  // TODO: Process the response from the Gemini API.
// //  processResponse(geminiResponse);
// //  client.stop();
// //  }
