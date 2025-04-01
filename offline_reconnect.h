void listen_for_hardreset()
{
  int buttonState = digitalRead(resetPin);
  if (buttonState == LOW) {
    if (!reset_button_pressed) {
      buttonPressStart = millis();
      reset_button_pressed = true;
    }
    if (millis() - buttonPressStart >= 8000) {

      for (int i = 0 ; i < 60  ; i++) {
        EEPROM.write(i, 0);
      }

      EEPROM.end();
      delay(100);
      StaticJsonDocument<2000> pids;
      pids["id"] = PId;
      serializeJson(pids, hardware_productid);
      client.publish(topic4, hardware_productid);
      delay(500);
      ESP.restart();
    }
  } else {
    reset_button_pressed = false;
  }
}
void reconnecting_wifi() {
  unsigned long reconnectionStartTime = millis();
  const unsigned long maxReconnectTime = 60000; // 1 minute

  while (WiFi.status() != WL_CONNECTED) {
   // p("Connecting.........");
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
  
    int buttonState = digitalRead(resetPin);
    if (buttonState == LOW) {
      if (!reset_button_pressed) {
        buttonPressStart = millis();
        reset_button_pressed = true;
      }
      if (millis() - buttonPressStart >= 8000) {
        // Factory reset process
        for (int i = 0; i < 60; i++) {
          EEPROM.write(i, 0);
        }
        EEPROM.end();
        ESP.restart();
        delay(100);
      }
    }

    // Check if reconnection time exceeds one minute
    if (millis() - reconnectionStartTime > maxReconnectTime) {
      //p("WiFi reconnection took too long. Disconnecting WiFi...");
      WiFi.disconnect(); // Disconnect WiFi
      break;
    }
  }
}
void reconnect()
{
  static unsigned long reconnectTime = 0;
  static int mqttConnectionState = 0;
  while (!client.connected()) {
    String ssid = readssid();
    String password = readpassword();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());
    reconnecting_wifi();
    if (WiFi.status() == WL_CONNECTED) {
      if (mqttConnectionState == 0) {
        if (client.connect(mqttClientId, mqttUsername, mqttPassword)) {
          client.subscribe(topic1, subscribeQoS);
          client.subscribe(topic2, subscribeQoS);
          client.subscribe(topic5, subscribeQoS);
          client.subscribe(topic6, subscribeQoS);
          client.publish(topic3, hardware_current_Status, false);
          client.publish(topic4, hardware_productid);
          client.publish(topic5, softreset_status);
          digitalWrite(led, HIGH);
         // p("MQTT connected.........");
        } else {
         // p("MQTT connection attempt failed, will retry in 5 seconds...");
          mqttConnectionState = 1;
          reconnectTime = millis() + 11000;  // Retry in 5 seconds
          digitalWrite(led, LOW);
        }
      }
      else if (mqttConnectionState == 1 && millis() >= reconnectTime) {
        mqttConnectionState = 0;  // Reset the state  
        digitalWrite(led, LOW);
      }
    }
  }
}
