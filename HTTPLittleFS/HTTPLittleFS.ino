//Download a file from a public URL and store that file in LittleFS in ESP32

//Create a data folder in the sketch folder.
//Create a file in the data folder and upload that file to the LittleFS using LittleFS Upload Data to ESP32 option when you click on ctrl+shift+p.

//Include the libraries
#include "WiFi.h"
#include "HTTPClient.h"
#include "FS.h"
#include "LittleFS.h"

// Your Wi-Fi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// The URL of the file you want to download
const char* file_url = "http://example.com/path/to/your/file.txt"; 

// The path where the file will be saved on LittleFS
const char* file_path = "/file.txt";

//READ THE FILE CONTENT

void readFile(const char * path) {
  Serial.printf("\nReading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("- Failed to open file for reading");
    return;
  }

  Serial.println("- File Content:");
  // Read and print the content line by line
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println("\n--- End of File ---");
  file.close();
}

// MAIN DOWNLOAD FUNCTION

void downloadFile() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot download.");
    return;
  }

  HTTPClient http;
  
  // 1. Begin the HTTP request
  Serial.printf("\nStarting download from: %s\n", file_url);
  http.begin(file_url); 

  // 2. Execute GET request
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      // 3. Open the file in LittleFS for writing
      File file = LittleFS.open(file_path, "w");
      if (!file) {
        Serial.println("Error: Failed to open file for writing to LittleFS.");
        http.end();
        return;
      }
      
      // Get the response stream pointer and content length
      WiFiClient* stream = http.getStreamPtr();
      stream->setTimeout(0); //Prevents unnecessary delays
      int contentLength = http.getSize();//Gets the size of the file
      Serial.printf("Response Code: %d (OK)\n", httpCode);
      Serial.printf("Content Length: %d bytes\n", contentLength);

      // 4. Stream the data chunk by chunk
      Serial.println("Downloading and streaming content...");
      size_t writtenBytes = 0;
      
      // Use a buffer for efficient writing and speed
      uint8_t buff[4096];
      
      while (http.connected() && (contentLength > 0 || contentLength == -1)) {
        size_t size = stream->available();
        if (size) {
          // Read up to 4096 bytes or the available size, whichever is smaller
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          
          // Write the read bytes to the file
          size_t bytesWritten = file.write(buff, c);
          writtenBytes += bytesWritten;
          
          if (contentLength > 0) {
            contentLength -= c;
          }
        }
      }
      
      // 5.Close the file to flush the buffer and save the content
      file.close(); 
      Serial.printf("Successfully downloaded file to LittleFS at: %s\n", file_path);
      Serial.printf("Total Bytes Written: %u\n", writtenBytes);
      
    } else {
      Serial.printf("HTTP GET failed. Error code: %d\n", httpCode);
    }
  } else {
    Serial.printf("HTTP Request failed. Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  // 6. Clean up the connection
  http.end(); 
}

// SETUP AND LOOP functions

void setup() {
  Serial.begin(115200);
  
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed. Formatting...");
    LittleFS.format();
    if (!LittleFS.begin(false)) {
      Serial.println("LittleFS re-mount failed! Stopping.");
      return;
    }
  }
  Serial.println("LittleFS Mounted Successfully.");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Perform the file download
  downloadFile();

  // Read the content
  readFile(file_path);
}

void loop() {
  // Nothing needed in the loop for this example
}