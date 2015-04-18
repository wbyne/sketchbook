

void setup () {
Serial.begin(9600);
pinMode(3,OUTPUT);
}

void loop () {
  digitalWrite (3, HIGH);
  Serial.println("HIGH");
  delay(5000);
  digitalWrite(3,LOW);
  Serial.println("LOW");
  delay(5000);
}
