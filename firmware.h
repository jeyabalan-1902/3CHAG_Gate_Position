void frimware_update(String firmwareUrl) {
 // p("Initating Frimware Updations!!!");
  delay(1000);
  if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection status
    //p("Network is perfect");
    HTTPClient http;  // Declare an object of class HTTPClient
    http.setTimeout(10000);  // Set timeout to 10 seconds
   // p("Contacting server at ");
    //p(firmwareUrl);
    http.begin(firmwareUrl);  // Specify request destination
   // p("Sending HTTP GET request");
    int httpCode = http.GET();  // Send the request
    //p("Status code = ");
    Serial.println(httpCode);
    delay(1000);

    if (httpCode == HTTP_CODE_OK) {  // Check the returning code
      // Debugging
     // p("HTTP Code is OK");

      int contentLength = http.getSize();
      Serial.printf("Content Length: %d\n", contentLength); // Debugging

      bool canBegin = Update.begin(contentLength);
     // p(" can begin = ");
      Serial.print(bool(canBegin));

      if (canBegin) {
        WiFiClient client = http.getStream();
        // Debugging
       // p("Beginning update");

        int written = Update.writeStream(client);
     //   p("written = ");
        Serial.println(written);

        if (written == contentLength) {
          //p("Successfully updated firmware!");
        } else {
          Serial.printf("Firmware update failed! Written: %d bytes\n", written);
        }
        if (Update.end()) {
          if (Update.isFinished()) {
           // p("Update successfully completed. Rebooting.");
            ESP.restart();
          } else {
            //p("Error occurred. Exiting.");
          }
        } else {
          Serial.printf("Error Occurred. Error #: %d\n", Update.getError());
        }
      } else {
       // p("Not enough space to begin OTA");
      }
    }
  }
}
