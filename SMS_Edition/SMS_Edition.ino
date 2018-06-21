#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <ESP8266WiFi.h> // Used for Wifi 
#include <PubSubClient.h> // Used for MQTT
#include <ArduinoJson.h> // Used for Json
#include <WiFiClientSecure.h>

//MQTT Setup
const char* ssid = "DESKTOP-78EVTDF 0993"; // Used for Wifi- wifi username
const char* password = "jayjay108"; // Used for Wifi - wifi password

const char* mqtt_server = "146.169.221.235"; // MQTT- IP address
const char* mqtt_password =""; // MQTT
const char* mqtt_topic = "Jason/AVDetection"; // MQTT
int mqttRetiresLeft = 10;
int wifiAttemptsLeft = 10;
WiFiClient espClient;
PubSubClient client(espClient);

//SMS Setup
String url = "/trigger/ESP/with/key/pSuf7t0GRtomn0sv0fBcm";
const char* host = "maker.ifttt.com";
const int httpsPort = 443;
WiFiClientSecure client1;

/* Assign a unique ID to this sensor at the same time */
//LSM303 Compass
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);
//LSM303 Accelemeter
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

//Multiplexer Setup with PulseReading/Soundcard.
int s0 = 2; // Brown
int s1 = 16; // White
int s2 = 0; // Blue
int common1 = 15;
int ss0 = 13;
int ss1 = 12;
int ss2 = 14;
int common = 17; // Pink
int x = 3;
int threshold = 30;
int prevvalue0 = 0;
int prevvalue45 = 0;
int prevvalue90 = 0;
int prevvalue135 = 0;
int prevvalue180 = 0;
int prevvalue225 = 0;
int prevvalue270 = 0;
int prevvalue315 = 0;
int sensor0count = 0;
int sensor45count = 0;
int sensor90count = 0;
int sensor135count = 0;
int sensor180count = 0;
int sensor225count = 0;
int sensor270count = 0;
int sensor315count = 0;
String sensor0;
String sensor45;
String sensor90;
String sensor135;
String sensor180;
String sensor225;
String sensor270;
String sensor315;
String detecteddir ="";
int maximumcount;
unsigned long currenttime;
unsigned long starttime;

//Variables for Compass
float Pi = 3.14159;
float bearing;

void setup(void) 
{
  Serial.begin(9600);
  mute();
  //Setup for LSM303
  //MQTT Setup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  //Pin Setup for Multiplexer
  pinMode(common,INPUT);
  pinMode(common1,OUTPUT);
  pinMode(ss0,OUTPUT);
  pinMode(ss1,OUTPUT);
  pinMode(ss2,OUTPUT);
  pinMode(s0,OUTPUT);
  pinMode(s1,OUTPUT);
  pinMode(s2,OUTPUT);
  pinMode(x,OUTPUT);
  analogWrite(x,512);
  analogWriteFreq(10000);
  currenttime = millis();
}
 
void loop() 
{
  //MQTT Connection
  if (!client.connected() && mqttRetiresLeft > 0) 
  {
    reconnect();
    mqttRetiresLeft --;
  }
  else if (mqttRetiresLeft > 0)
  {
    mqttRetiresLeft = 10;
    client.loop();
  }

  compass();
  crashdetect();

  //Multiplexer calling
  sensor0 = readSensorPulse(1,0,1,prevvalue0);// Number refers to the bearing of the helmet going clockwise
  sensor45 = readSensorPulse(1,1,1,prevvalue45);
  sensor90 = readSensorPulse(1,1,0,prevvalue90);
  sensor135 = readSensorPulse(1,0,0,prevvalue135);
  sensor180 = readSensorPulse(0,1,0,prevvalue180);
  sensor225 = readSensorPulse(0,0,1,prevvalue225);
  sensor270 = readSensorPulse(0,0,0,prevvalue270);
  sensor315 = readSensorPulse(0,1,1,prevvalue315);
  
  if (sensor0 == "Detected")
  {
    sensor0count++;
  }
  if (sensor45 == "Detected")
  {
    sensor45count++;
  }
  if (sensor90 == "Detected")
  {
    sensor90count++;
  }  
  if (sensor135 == "Detected")
  {
    sensor135count++;
  }  
  if (sensor180 == "Detected")
  {
    sensor180count++;
  }
  if (sensor225 == "Detected")
  {
    sensor225count++;
  }
  if (sensor270 == "Detected")
  {
    sensor270count++;
  }
  if (sensor315 == "Detected")
  {
    sensor315count++;
  }
  starttime = millis();
  if(starttime - currenttime > 1500)
  {
    currenttime = millis();
    maximumcount =max(sensor0count,sensor45count);
    maximumcount =max(maximumcount,sensor90count);
    maximumcount =max(maximumcount,sensor135count);
    maximumcount =max(maximumcount,sensor180count);
    maximumcount =max(maximumcount,sensor225count);
    maximumcount =max(maximumcount,sensor270count);
    maximumcount =max(maximumcount,sensor315count);
    if(maximumcount ==0)
    {
      detecteddir = "Not detected";
    }
    else if (maximumcount = sensor0count)
    {
      detecteddir = "Detected at 0";
    }
    else if (maximumcount = sensor45count)
    {
      detecteddir = "Detected at 45";
    }
    else if (maximumcount = sensor90count)
    {
      detecteddir = "Detected at 90";
    }
    else if (maximumcount = sensor135count)
    {
      detecteddir = "Detected at 135";
    }
    else if (maximumcount = sensor180count)
    {
      detecteddir = "Detected at 180";
    }
    else if (maximumcount = sensor225count)
    {
      detecteddir = "Detected at 225";
    }
    else if (maximumcount = sensor270count)
    {
      detecteddir = "Detected at 270";
    }
    else if (maximumcount = sensor315count)
    {
      detecteddir = "Detected at 315";
    }
    sensor0count = 0;
    sensor45count = 0;
    sensor90count = 0;
    sensor135count = 0;
    sensor180count = 0;
    sensor225count = 0;
    sensor270count = 0;
    sensor315count = 0;

    alert(detecteddir);
    char charBuf[256];
    detecteddir.toCharArray(charBuf,256);
    client.publish(mqtt_topic, charBuf , true);
    Serial.println(detecteddir);
  }
  
  delay(100);

}

void setup_wifi() //Working Wifi
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && wifiAttemptsLeft > 0) 
  {
    delay(500);
    Serial.print(".");
    wifiAttemptsLeft --;
  }

  if (WiFi.status() == WL_CONNECTED){
    wifiAttemptsLeft = 10;
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()&& mqttRetiresLeft > 0) 
  {
    Serial.print("Attempting MQTT connection...");

    mqttRetiresLeft --;
    
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      
    } else 
      {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      // Wait 5 seconds before retrying
      delay(500);
      }
  }
}

String readSensorPulse(int b2 , int b1 , int b0, int prevvalue){

  digitalWrite(s2,b2);
  digitalWrite(s1,b1);
  digitalWrite(s0,b0);
  int value = analogRead(common);
  bool spike;

  
  if(value - prevvalue > threshold)
  {
    spike = true;
  }
  else
  {
    spike = false;
  }
  if(b2 == 1 && b1 == 0 && b0 == 1)
    {
      prevvalue0 = value;
    }
    if(b2 == 1 && b1 == 1 && b0 == 1)
    {
      prevvalue45 = value;
    }
    if(b2 == 1 && b1 == 1 && b0 == 0)
    {
      prevvalue90 = value;
    }
    if(b2 == 1 && b1 == 0 && b0 == 0)
    { 
      prevvalue135 = value;
    }
    if(b2 == 0 && b1 == 1 && b0 == 0)
    {
      prevvalue180 = value;
    }
    if(b2 == 0 && b1 == 0 && b0 == 1)
    {
      prevvalue225 = value;
    }
    if(b2 == 0 && b1 == 0 && b0 == 0)
    {
      prevvalue270 = value; 
    }
    if(b2 == 0 && b1 == 1 && b0 == 1)
    {
      prevvalue315 = value;
    }
  
    if(spike == true)
    {
      return "Detected"; 
    }
    else
    {
      return "Not Detected";
    }
   
}

void compass()
{
  //Compass 
  sensors_event_t event; 
  mag.getEvent(&event);
  // Calculate the angle of the vector y,x
  float bearing = (atan2((event.magnetic.y+23.5),(event.magnetic.x-12.85)))*180/Pi;
  
  // Normalize to 0-360 and count for offset from the helmet positioning
  if (bearing < 0)
  {
    bearing = (450 + bearing);
    if(bearing > 360)
    {
    bearing = bearing - 360;
    }
  }
  else
  {
    bearing = bearing + 90;
  }
}

void crashdetect()
{
  sensors_event_t event;
  accel.getEvent(&event);
  if(abs(event.acceleration.z) > 17)
  {
  client.publish(mqtt_topic, "Crashed" , true);
  Serial.println("Crashed");
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client1.connect(host, httpsPort)) 
  {
    Serial.println("connection failed");
  }
  Serial.print("requesting URL: ");
  Serial.println(url);
  client1.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client1.readStringUntil('\n');
    if (line == "\r") 
    {
      Serial.println("headers received");
      break;
    }
  }
  String line = client1.readStringUntil('\n');

  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  delay(5000);
  }
}
void alert(String detecteddir)
{
    if(detecteddir == "Detected at 0"){playsound(0,0,0);}
    if(detecteddir == "Detected at 45"){playsound(0,0,1);}
    if(detecteddir == "Detected at 90"){playsound(0,1,0);}
    if(detecteddir == "Detected at 135"){playsound(0,1,1);}
    if(detecteddir == "Detected at 180"){playsound(1,0,0);}
    if(detecteddir == "Detected at 225"){playsound(1,0,1);}
    if(detecteddir == "Detected at 270"){playsound(1,1,0);}
    if(detecteddir == "Detected at 315"){playsound(1,1,1);}
    char charBuf[256];
    detecteddir.toCharArray(charBuf,256);
    client.publish(mqtt_topic, charBuf , true);
    Serial.println(detecteddir);
}

void playsound(int c2,int c1,int c0)
{
  digitalWrite(ss0,c0);
  digitalWrite(ss1,c1);
  digitalWrite(ss2,c2);
  digitalWrite(common1,LOW);
  delay(200);
  digitalWrite(common1,HIGH);
}

void mute()
{
  digitalWrite(ss0,0);
  digitalWrite(ss1,0);
  digitalWrite(ss2,0);
  delay(10);
  digitalWrite(common1,HIGH);
  digitalWrite(ss0,0);
  digitalWrite(ss1,0);
  digitalWrite(ss2,1);
  delay(10);
  digitalWrite(common1,HIGH);
  digitalWrite(ss0,0);
  digitalWrite(ss1,1);
  digitalWrite(ss2,0);
  delay(10);
  digitalWrite(common1,HIGH);
  digitalWrite(ss0,0);
  digitalWrite(ss1,1);
  digitalWrite(ss2,1);
  delay(10);
  digitalWrite(common1,HIGH);  
  digitalWrite(ss0,1);
  digitalWrite(ss1,0);
  digitalWrite(ss2,0);
  delay(10);
  digitalWrite(common1,HIGH);
  digitalWrite(ss0,1);
  digitalWrite(ss1,0);
  digitalWrite(ss2,1);
  delay(10);
  digitalWrite(common1,HIGH);
  digitalWrite(ss0,1);
  digitalWrite(ss1,1);
  digitalWrite(ss2,0);
  delay(10);
  digitalWrite(common1,HIGH);
  digitalWrite(ss0,1);
  digitalWrite(ss1,1);
  digitalWrite(ss2,1);
  delay(10);
  digitalWrite(common1,HIGH);
}

