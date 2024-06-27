#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W) || defined(ARDUINO_GIGA)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>) || defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#elif __has_include(<WiFiC3.h>) || defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif __has_include(<WiFi.h>)
#include <WiFi.h>
#endif

#include <FirebaseClient.h>

#define WIFI_SSID "ICHA"
#define WIFI_PASSWORD "jakarta682"
#define DATABASE_URL "crudmedlist-8b416-default-rtdb.firebaseio.com"

DefaultNetwork network;
NoAuth no_auth;
FirebaseApp app;

#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_GIGA) || defined(ARDUINO_PORTENTA_C33) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiSSLClient.h>
WiFiSSLClient ssl_client;
#endif

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
RealtimeDatabase Database;

bool taskComplete = false;

#include <HardwareSerial.h> //UART LIB
HardwareSerial SerialPort(2); //UART 2 (GPIO 16 DAN 17)
bool serialport = false; 

#include <L298NX2.h>
const unsigned int EN_A = 13;
const unsigned int IN1_A = 12;
const unsigned int IN2_A = 14;

const unsigned int IN1_B = 27;
const unsigned int IN2_B = 26;
const unsigned int EN_B = 25;
L298NX2 motors(EN_A, IN1_A, IN2_A, EN_B, IN1_B, IN2_B);

int drop1 = 0, drop2 = 0, drop3 = 0; 
int pick1 = 0, pick2 = 0, pick3 = 0;
const int pompa =  22;  //pompa 

const int hP1 = 34; // Hall sensor home
const int hP2 = 35;  // Hall sensor satu
const int hP3 = 32;  // Hall sensor kedua
const int hP4 = 19;  // Hall sensor ketiga

int hS1 = 0 , hS2 = 1, hS3 = 1, hS4 = 1; 

String iv, k, m1, m2, m3;
int MS1[] = {1,2,3};
int MS2[] = {1,2,3};
int MS3[] = {1,2,3};
int M1, M2, M3;
int ind1, ind2, ind3,s;


int BTN;
int Q1, Q2, Q3;
int newQ1, newQ2, newQ3;
int W;

void setup()
{
    Serial.begin(115200);
      pinMode(hP1, INPUT);
      pinMode(hP2, INPUT);
      pinMode(hP3, INPUT);
      pinMode(hP4, INPUT);
      pinMode(pompa, OUTPUT);

  // Set initial speed for both motors
  motors.setSpeed(60); // motor dc 
  digitalWrite(pompa, HIGH);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    SerialPort.begin(115200, SERIAL_8N1, 16, 17);//intializating UART
    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    Serial.println("Initializing app...");

#if defined(ESP32) || defined(ESP8266) || defined(PICO_RP2040)
    ssl_client.setInsecure();
#if defined(ESP8266)
    ssl_client.setBufferSizes(4096, 1024);
#endif
#endif

    initializeApp(aClient, app, getAuth(no_auth));
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
}

void loop()
{
    // The async task handler should run inside the main loop
    // without blocking delay or bypassing with millis code blocks.
    hS1 = digitalRead(hP1);
    hS2 = digitalRead(hP2);
    hS3 = digitalRead(hP3);
    hS4 = digitalRead(hP4);

    app.loop();
    Database.loop();
    if (app.ready() && !taskComplete)
    {
        taskComplete = true;
        Database.get(aClient, "/meds/1/quantity", asyncCB1);
        Database.get(aClient, "/meds/2/quantity", asyncCB2);
        Database.get(aClient, "/meds/3/quantity", asyncCB3);
        Database.get(aClient, "/schedules/MS1", asyncCB4);
        Database.get(aClient, "/schedules/MS2", asyncCB5);
        Database.get(aClient, "/schedules/MS3", asyncCB6);
    }

    if(SerialPort.available() > 0){
        BTN = SerialPort.parseInt(); //ini ditaro di dlm app ready karena nanti ada update q1,q2 firebase
        Serial.print("BTN = ");
        Serial.println(BTN);
        delay(5000);
        if(BTN == 1){ //jika alarm 1 teresekusi di esp1 maka btn == 1 dst
          for(int i = 0; i<sizeof(MS1) / sizeof(MS1[0]); i++)
          {
          Serial.print("  Medicine No. ");
          Serial.println(MS1[i]);
          W = MS1[i];
          work(); // medicine no.
          }
          //resetflag
        } //btn1
        if(BTN == 2){
          for(int i = 0; i<sizeof(MS2) / sizeof(MS2[0]); i++)
          {
          Serial.print("  Medicine No. ");
          Serial.println(MS2[i]);
          W = MS2[i];
          work(); // medicine no.
          }
          //resetflag
        }//btn2
        if(BTN == 3){
          for(int i = 0; i<sizeof(MS3) / sizeof(MS3[0]); i++)
          {
          Serial.print("  Medicine No. ");
          Serial.println(MS3[i]);
          W = MS3[i];
          work(); // MEKANIKAL
          }
          //resetflag
        }//btn3
    }   //serialportavail
}//loop

void asyncCB1(AsyncResult &aResult){
  if (aResult.available())
    {
      Q1 = atoi(aResult.c_str());
      //Serial.print("Q1 = ");
      //Serial.println(Q1);
    }
}

void asyncCB2(AsyncResult &aResult){
  if (aResult.available())
    {
      Q2 = atoi(aResult.c_str());
      //Serial.print("Q2 = ");
      //Serial.println(Q2);
    }
}

void asyncCB3(AsyncResult &aResult){
    if (aResult.available())
    {
      Q3 = atoi(aResult.c_str());
      //Serial.print("Q3 = ");
      //Serial.println(Q3);
    }
}

void asyncCB4(AsyncResult &aResult){
    if (aResult.available())
    {
    k = aResult.c_str();
    int s = k.indexOf('[');
    ind1 = k.indexOf(',');           m1 = k.substring(s+1, ind1);      M1 = m1.toInt(); MS1[0]=M1; 
    ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS1[1]=M2; 
    ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS1[2]=M3; 
    }
}

void asyncCB5(AsyncResult &aResult){
    if (aResult.available())
    {
    k = aResult.c_str();
    s = k.indexOf('[');
    ind1 = k.indexOf(',');           m1 = k.substring(s+1, ind1);      M1 = m1.toInt(); MS2[0]=M1; 
    ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS2[1]=M2; 
    ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS2[2]=M3; 
    }
}

void asyncCB6(AsyncResult &aResult){
    if (aResult.available())
    {
    k = aResult.c_str();
     s = k.indexOf('[');
    ind1 = k.indexOf(',');           m1 = k.substring(s+1, ind1);      M1 = m1.toInt(); MS3[0]=M1; 
    ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS3[1]=M2; 
    ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS3[2]=M3; 
    }
}

void work(){
  hS1 = digitalRead(hP1);
  hS2 = digitalRead(hP2);
  hS3 = digitalRead(hP3);
  hS4 = digitalRead(hP4);
  if (W == 0){}
  else if (W == 1)
  { 
        if(hS1 == 0 && drop1 == 0){ //default awal home 0 ke kompartemen berubah jd 1 
        digitalWrite(pompa, HIGH);
        motors.forwardA();
        while (!(digitalRead(hP2) == 0)) {
            // Tunggu hingga kondisi hS1 == 1 dan hS3 == 0 terpenuhi
        }
        motors.stop(); 
          while (digitalRead(hP2) == 0 && pick1 == 0){
              digitalWrite(pompa, LOW); //pompanyala
              motors.backwardB();//TURUN KOMPARTEMEN
              delay(4000);
              motors.stop();
              delay(3000);
              motors.forwardB();//TURUN KOMPARTEMEN
              delay(4000);
              motors.stop();
              break;
              pick1 = 1;
          }

          motors.backwardA();
          while(!(digitalRead(hP1) == 0)){

          }
          motors.stopA();
          delay(4000);
          digitalWrite(pompa, HIGH); 
          drop1 = 1; 
      }//ifhs==0
  int drop1 = 0, drop2 = 0, drop3 = 0; 
  int pick1 = 0, pick2 = 0, pick3 = 0;

  }//w=1

  else if(W == 2)
  { 
        if(hS1 == 0 && drop2 == 0){ //default awal home 0 ke kompartemen berubah jd 1 
        digitalWrite(pompa, HIGH);
        motors.forwardA();
        while (!(digitalRead(hP3) == 0)) {
            // Tunggu hingga kondisi hS1 == 1 dan hS3 == 0 terpenuhi
        }
        motors.stop(); 
          while (digitalRead(hP3) == 0 && pick2 == 0){
              digitalWrite(pompa, LOW); //pompanyala
              motors.backwardB();//TURUN KOMPARTEMEN
              delay(4000);
              motors.stop();
              delay(3000);
              motors.forwardB();//TURUN KOMPARTEMEN
              delay(4000);
              motors.stop();
              break;
              pick2 = 1;
          }

          motors.backwardA();
          while(!(digitalRead(hP1) == 0)){g

          }
          motors.stopA();
          delay(4000);
          digitalWrite(pompa, HIGH); 
          drop2 = 1; 
      }//ifhs==0
    int drop1 = 0, drop2 = 0, drop3 = 0; 
    int pick1 = 0, pick2 = 0, pick3 = 0;

  }//w=2
  
  else if(W == 3)
  { 

        if(hS1 == 0 && drop1 == 0){ //default awal home 0 ke kompartemen berubah jd 1 
        digitalWrite(pompa, HIGH);
        motors.forwardA();
        while (!(digitalRead(hP4) == 0)) {
            // Tunggu hingga kondisi hS1 == 1 dan hS3 == 0 terpenuhi
        }
        motors.stop(); 
          while (digitalRead(hP4) == 0 && pick3 == 0){
              digitalWrite(pompa, LOW); //pompanyala
              motors.backwardB();//TURUN KOMPARTEMEN
              delay(4000);
              motors.stop();
              delay(3000);
              motors.forwardB();//TURUN KOMPARTEMEN
              delay(4000);
              motors.stop();
              break;
              pick3 = 1;
          }

          motors.backwardA();
          while(!(digitalRead(hP1) == 0)){

          }
          motors.stopA();
          delay(4000);
          digitalWrite(pompa, HIGH); 
          drop3 = 1; 
      }//ifhs==0
  }//w=3

int drop1 = 0, drop2 = 0, drop3 = 0; 
int pick1 = 0, pick2 = 0, pick3 = 0;

}//work


// void updateQ1(){
// updaate path /meds/1/quantity dgn nilai quantity lama dikurang 1 jika obat terambil 1
// }
// void updateQ2(){
// updaate path /meds/2/quantity dgn nilai quantity lama dikurang 1 jika obat terambil 1
// }
// void updateQ3(){
// updaate path /meds/3/quantity dgn nilai quantity lama dikurang 1 jika obat terambil 1
// }