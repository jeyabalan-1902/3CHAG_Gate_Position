#include <WiFi.h>                    // library
#include <Wire.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Update.h>
#include <PubSubClient.h>

#include"mqtt_topic.h"
#include"initial_function.h"
#include"firmware.h"
#include"mqtt_fun.h"  
#include"eeprom_storage.h"
#include"offline_reconnect.h"

#define ADC1_PIN 34  
#define ADC2_PIN 35  
#define ADC3_PIN 36  
#define ADC4_PIN 39 

int read_adc_smooth(int pin);
String evaluateGateStatus(String last_status);

void Gate_position();
char gate_status[200];
String last_status = ""; 
String current_status = "";

 int adc1_value, adc2_value, adc3_value, adc4_value;
 float voltage1, voltage2, voltage3, voltage4;  

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
  Gate_position();
  client.loop(); 
  
}

void Gate_position() {
  String temp_status = evaluateGateStatus(last_status);
  delay(2000); // Wait for relays to settle
  String final_status = evaluateGateStatus(last_status);

  if (final_status == temp_status && final_status != last_status) 
  {
    Serial.println("Gate is " + final_status);
    
    StaticJsonDocument<2000> gate;
    gate["status"] = final_status;
    serializeJson(gate, gate_status);
    client.publish(topic7, gate_status);

    last_status = final_status;
  }
}

String evaluateGateStatus(String last_status) {
    int adc1_value = read_adc_smooth(ADC1_PIN);
    int adc2_value = read_adc_smooth(ADC2_PIN);
    int adc3_value = read_adc_smooth(ADC3_PIN);
    int adc4_value = read_adc_smooth(ADC4_PIN);

    float voltage1 = (adc1_value / 4095.0) * 3.3;
    float voltage2 = (adc2_value / 4095.0) * 3.3;
    float voltage3 = (adc3_value / 4095.0) * 3.3;
    float voltage4 = (adc4_value / 4095.0) * 3.3;

    Serial.print("ADC34: ");
    Serial.print(voltage1, 3);
    Serial.print("V | ADC35: ");
    Serial.print(voltage2, 3);
    Serial.print("V | ADC12: ");
    Serial.print(voltage3, 3);
    Serial.print("V | ADC13: ");
    Serial.print(voltage4, 3);
    Serial.println("V");

    if ((voltage1 > 0.9 && voltage2 < 0.3) && (voltage3 > 0.9 && voltage4 < 0.3)) {
        current_status = "DOUBLE_GATE_OPENING";
        return current_status;
    }
    else if ((voltage1 < 0.3 && voltage2 > 0.9) && (voltage3 < 0.3 && voltage4 > 0.9)) {
        current_status = "DOUBLE_GATE_CLOSING";
        return current_status;
    }
    else if ((voltage1 > 0.6 && voltage2 < 0.3) && (voltage3 > 0.6 && voltage4 > 0.6)) {
        current_status = "SINGLE_GATE_OPENING";
        return current_status;
    }
    else if ((voltage1 > 0.4 && voltage2 > 0.4) && (voltage3 < 0.3 && voltage4 > 0.4)) {
        current_status = "SINGLE_GATE_CLOSING";
        return current_status;
    }
    else {
        last_status = current_status;
        if (last_status == "DOUBLE_GATE_OPENING" || last_status == "SINGLE_GATE_OPENING")
        {
            return "GATE_OPENED";   
        }
        else if(last_status == "DOUBLE_GATE_CLOSING" || last_status == "SINGLE_GATE_CLOSING") 
        {
            return "GATE_CLOSED";    
        } 
       

    }
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

