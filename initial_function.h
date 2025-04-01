int R1 = 26;
int R2 = 25;  //OUTPUT
int R3 = 33;

int led = 4;  //4//22;//led

const int publishQoS = 1;
const int subscribeQoS = 1;
 
int device1;
int device2;
int device3;

const int resetPin = 32;
unsigned long buttonPressStart = 0;
bool reset_button_pressed = false;

void intializepins() {
  pinMode(resetPin, INPUT);

  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);

  pinMode(led, OUTPUT);

  digitalWrite(R1, LOW);
  digitalWrite(led, LOW);
  digitalWrite(R2, LOW);
  digitalWrite(R3, LOW);
}
void blinkled3time() {
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);
}
