char softreset_status[200];
char ssid[32];
char password[64];
String store;

const char* mqttServer = "mqtt.onwords.in";// mqttt
const int mqttPort = 1883;
const char* mqttUsername = "Nikhil";
const char* mqttPassword = "Nikhil8182";

char hardware_current_Status[300];
char hardware_productid[100];
extern String current_status;
extern char gate_status[200];

extern void Gate_position();
extern void writePid(const String &Pid);

WiFiServer server(8182);
WiFiClient espClient;
PubSubClient client(espClient);

void paringMode()
{
  WiFi.mode(WIFI_AP);
  String hotspot = "onwords-" + PId; //hotsopt name with pid
  const char* hotspot_name = hotspot.c_str();// hotspot creation
  WiFi.softAP(hotspot_name, "");  // assigning name to hotspot
  server.begin();                 //starting http server
  WiFi.softAPIP();
  Serial.println(WiFi.softAPIP());//print webserver local ip by default 192.168.4.1
  delay(2000);                    //this is mandatory delay
}

void currentstatus( String store)
{

  StaticJsonDocument<2000> doc;
  doc["action"] = store;
  serializeJson(doc, hardware_current_Status);
  client.publish(topic3, hardware_current_Status, false);
}
void callback(char* topic, byte * payload, unsigned int length) {
  Serial.print(topic);
  String aa = "";
  for (int i = 0; i < length; i++) {
    aa += (char)payload[i];
  }
  Serial.println(aa);
  DynamicJsonDocument doc(6000);
  DeserializationError error = deserializeJson(doc, aa); // Deserialize the message into the JSON document
  if (error) {
    return;
  }
  int firmware_sts = doc["firmware_sts"];
  if (firmware_sts == 1)
  {
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["action"] = "Firmware Update";
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);
    client.publish(topic6, jsonBuffer);
    delay(2000);
    String firmwareUrl = doc["firmwareUrl"];
    frimware_update(firmwareUrl);
  }
  String action = doc["action"];
  Serial.println(action);
  if ( action == "singleGate")
  {
    Serial.println("singleGate");
    digitalWrite(R1, HIGH);
    delay(600);
    digitalWrite(R1, LOW);
    store = "singleGate";
    currentstatus(store);
  }

  if ( action == "doubleGate")
  {

    Serial.println("doubleGate");
    digitalWrite(R2, HIGH);
    digitalWrite(R3, HIGH);
    delay(600);
    digitalWrite(R2, LOW);
    digitalWrite(R3, LOW);
    store = "doubleGate";
    currentstatus(store);
  }
  
  if (doc.containsKey("PID")) {
    const char* pid = doc["PID"];
    Serial.println("Extracted PID: " + String(pid));

    writePid(pid);
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["action"] = "PID";
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);
    client.publish(topic8, jsonBuffer);
    Serial.println("PID stored in EEPROM!");

    delay(2000);  // Ensure stability before restart
    ESP.restart(); 
  }

  String response = doc["request"];
  Serial.println(response);

  if (response == "getCurrentStatus")
  {
    //String stored_status = readEEPROM(EEPROM_STATUS_ADDR, 10);
    // if(stored_status == "OPEN" || stored_status == "CLOSE")
    // {
    //   StaticJsonDocument<2000> gate;
    //   gate["status"] = current_status;
    //   serializeJson(gate, gate_status);
    //   client.publish(topic7, gate_status);
    // }

    StaticJsonDocument<2000> doc;
    doc["action"] = store;
    serializeJson(doc, hardware_current_Status);
    client.publish(topic3, hardware_current_Status, false);
  }
  int soft_resets = doc["reset"];
  if (soft_resets == 1)
  {
    for (int i = 0 ; i < 60  ; i++)
    {
      EEPROM.write(i, 0);
    }
    EEPROM.end();
    delay(100);
    StaticJsonDocument <2000>soft;
    soft["status"] = true;
    serializeJson(soft, softreset_status);
    client.publish(topic5, softreset_status);
    delay(500);
    ESP.restart();
  }
}
