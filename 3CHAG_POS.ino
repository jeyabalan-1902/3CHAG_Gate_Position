#include <WiFi.h>                    // library
#include <Wire.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Update.h>
#include <PubSubClient.h>

#include"print.h"
#include"mqtt_topic.h"
#include"initial_function.h"
#include"firmware.h"
#include"AT24C32N_EEPROM.h"
#include"mqtt_fun.h"  
#include"eeprom_storage.h"
#include"offline_reconnect.h"

#define ADC1_PIN 34  // GPIO34 (O1)
#define ADC2_PIN 35  // GPIO35 (O3)
#define ADC3_PIN 12
#define ADC4_PIN 13


int read_adc_smooth(int pin);
void Gate_position();
char gate_status[200];
String last_status = ""; 
String current_status = "";

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  analogReadResolution(12);
  EEPROM.begin(EEPROM_SIZE);
  String ssid = readssid();
  String password = readpassword() ;
  // p("ssid = " + ssid);
  // p("password = " + password);
  PId = readPid();
  //p("PId=" + PId);
  delay(1000);
  intializepins();
  if (ssid == "")
  {
    blinkled3time();  // will work only one time its a bug
    paringMode();
  }
  else
  {
    WiFi.begin(ssid.c_str(), password.c_str());
    reconnecting_wifi();
    digitalWrite(led, HIGH);
    parinMode = false;
    delay(200);
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    mqtt_topics();
  }
}

void loop()
{
  if (parinMode)
  {
    clients = server.available(); // need
    if (!clients) { 
      return;
    }
    while (!clients.available()) {
      delay(1);  
    }
    delay(2000);
    pair();
  }
  else
  {
    listen_for_hardreset();
  }
  if (!client.connected())
  {
    reconnect();
  }
  client.loop(); 
  Gate_position();
}

void Gate_position() {
    int adc1_value = read_adc_smooth(ADC1_PIN);
    int adc2_value = read_adc_smooth(ADC2_PIN);
    int adc3_value = read_adc_smooth(ADC3_PIN);
    int adc4_value = read_adc_smooth(ADC4_PIN);

    float voltage1 = (adc1_value / 4095.0) * 3.3;  
    float voltage2 = (adc2_value / 4095.0) * 3.3;
    float voltage3 = (adc3_value / 4095.0) * 3.3;
    float voltage4 = (adc4_value / 4095.0) * 3.3;


    if ((voltage1 > 0.3 && voltage2 < 0.3) || (voltage3 > 0.3 && voltage4 < 0.3)) {
        current_status = "CLOSE";
    } else if ((voltage2 > 0.3 && voltage1 < 0.3) || (voltage4 > 0.3 && voltage3 < 0.3)) {
        current_status = "OPEN";
    } else {
        current_status = "PAUSE";
    }

    if (current_status != last_status) {
        Serial.println("Gate is " + current_status);
        
        StaticJsonDocument<2000> gate;
        gate["status"] = current_status;
        serializeJson(gate, gate_status);
        client.publish(topic7, gate_status);
        if (current_status == "OPEN" || current_status == "CLOSE")
        {
          writeEEPROM(EEPROM_STATUS_ADDR, current_status);
        }
        last_status = current_status;  
    }

    delay(1000);
}

int read_adc_smooth(int pin) {
    int sum = 0;
    int samples = 10;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(2); 
    }
    return sum / samples; 
}
