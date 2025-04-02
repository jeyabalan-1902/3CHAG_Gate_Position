#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASSWORD_ADDR 30
#define PID_ADDR 60
#define RESULT_ADDR 100
bool parinMode = true;
WiFiClient clients;
String readssid() {
  String ssid;
  for (int i = SSID_ADDR; i < PASSWORD_ADDR; i++) {
    char character = EEPROM.read(i);
    if (character == 0) {
      break; // Null terminator, end of string
    }
    ssid += character;
  }
  return ssid;
}

String readpassword() {
  String password;
  for (int i = PASSWORD_ADDR; i < PID_ADDR; i++) {
    char character = EEPROM.read(i);
    if (character == 0) {
      break;
    }
    password += character;
  }
  return password;
}
String readPid() {
  String Pid;
  for (int i = PID_ADDR; i < RESULT_ADDR; i++) {
    char character = EEPROM.read(i);
    if (character == 0) {
      break;
    }
    Pid += character;
  }
  return Pid;
}

void writePid(const String &Pid) {
  int addr = PID_ADDR;  
  int len = Pid.length();
  for (int i = 0; i < len; i++) {
    EEPROM.write(addr + i, Pid[i]);
  }
  EEPROM.write(addr + len, 0);
  EEPROM.commit();  
}

void writestring(String data) {
  int separatorIndex = data.indexOf(':');
  if (separatorIndex != -1) {
    String ssid = data.substring(0, separatorIndex);
    String password = data.substring(separatorIndex + 1);

    if (PASSWORD_ADDR + ssid.length() + password.length() <= EEPROM_SIZE) {
      for (int i = 0; i < ssid.length(); i++) {
        EEPROM.write(SSID_ADDR + i, ssid[i]);
      }
      for (int i = 0; i < password.length(); i++) {
        EEPROM.write(PASSWORD_ADDR + i, password[i]);
      }
      EEPROM.commit();
    }
  }
}
void pair()
{
  String jsonPayload = clients.readString();
  int bodyStart = jsonPayload.indexOf("\r\n\r\n") + 4;
  String body = jsonPayload.substring(bodyStart);
  clients.flush();
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, body);
  const char* ssid = doc["ssid"].as<const char*>();
  const char* password = doc["password"].as<const char*>();
  // p("SSID: " + String(ssid));
  // p("Password: " + String(password));
  // p("HTTP/1.1 200 OK");
  String x = "connecting to " + String(ssid);
  clients.println("Content-Type: text/plain");
  clients.println("");
  clients.println("ok");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting....");
    digitalWrite(led, HIGH);
    delay(150);
    digitalWrite(led, LOW);
    delay(150);
  }
  String result = ssid + String(':') + password;
  writestring(result);
 // p("successfully written");
  digitalWrite(led, HIGH);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  mqtt_topics();
  parinMode = false;
}
