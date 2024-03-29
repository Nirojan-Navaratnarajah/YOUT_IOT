/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com

  link where code is reffered from : https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
                                     https://create.arduino.cc/projecthub/abdularbi17/ultrasonic-sensor-hc-sr04-with-arduino-tutorial-327ff6
                                     https://sensorkit.joy-it.net/de/sensors/ky-012
In the CLI of the Raspberry pi : 

    run : mosquitto_sub -t "esp32/buzzer"  //to see the state of buzzer.
     run : mosquitto_sub -t "esp32/ultra"  //to see if there is an object near 10cm.
      run : mosquitto_sub -t "#"  //to subscribe to all the topics.
       run : mosquitto_pub -t "esp32/output"  //to activate the buzzer.

       Code modified by : Neeroyan
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#define echoPin 33 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin  32 //attach pin D3 Arduino to pin Trig of HC-SR04

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
int Buzzer = 13;

// Replace the next variables with your SSID/Password combination
const char* ssid = "YOUR SSID"; // mobile hotspot
const char* password = "PASSWORD";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.80.1";   //--- IP address of Raspbery pi

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
 // Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
   pinMode (Buzzer, OUTPUT); // Initialisierung Ausgangspin für den Buzzer
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void buzzer();
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic esp32/output, you check if the message is "on" and then activate the buzzer . 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") { // topic to controll buzzer
    Serial.print("activating buzzer ");
    if(messageTemp == "on"){
      Serial.println("activating buzzer");
      buzzer();
    }
  }
}void ultra() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(500);
}
void buzzer () 
{
  digitalWrite (Buzzer, HIGH); // Buzzer wird eingeschaltet
  delay (500); // Wartemodus für 4 Sekunden
  digitalWrite (Buzzer, LOW); // Buzzer wird ausgeschaltet
  delay (500); // Wartemodus für weitere zwei Sekunden in denen die LED dann ausgeschaltet ist
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();

  ultra();
  if (now - lastMsg > 500) {
    lastMsg = now;
    
    Serial.print(distance);
    if(distance<10){
      buzzer();
       client.publish("esp32/buzzer", "buzzer on");//publishing data via MQTT

    
    client.publish("esp32/ultra", "Hand near sensor detected");}
    else 
    {client.publish("esp32/buzzer", "buzzer off");

    client.publish("esp32/ultra", "no object near sensor");}
  }
}
