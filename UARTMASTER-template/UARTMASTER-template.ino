#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Wire.h> 
#include <RtcDS3231.h>  
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h> //UART LIB
HardwareSerial SerialPort(2); //UART 2 (GPIO 16 DAN 17)

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define WIFI_SSID "sira" 
#define WIFI_PASSWORD "otsira116" 
#define API_KEY "AIzaSyAx7gQ7QVpawuz9FgpqT6bU0Wsz2hmhNsg" 
#define DATABASE_URL "crudmedlist-8b416-default-rtdb.firebaseio.com"  

#define buz  13
#define but  34

bool buzOn = false; // Penanda apakah buzzer sedang aktif atau tidak
bool serialport1 = false, serialport2 = false,  serialport3 = false; 
int btn;
int a1 = 0;
int a2 = 0;
int a3 = 0;

RtcDS3231<TwoWire> Rtc(Wire); 

FirebaseData fbdo;
FirebaseData stream,  stream_M1, stream_M2, stream_M3, stream_AH1, stream_AH2, stream_AH3, stream_AM1, stream_AM2, stream_AM3, stream_qty1, stream_qty2, stream_qty3;
FirebaseAuth auth;
FirebaseConfig config;

int AV1, AV2, AV3, pDay;
int Years, Months, Days, Hours, Min, Sec, date;
String waktu, timePath, iv; 

int Q1, Q2, Q3;

String k, m1, m2, m3; 

int MS1[] = {1,2,3};
int MS2[] = {1,2,3};
int MS3[] = {1,2,3};


int M1, M2, M3; 
int ind1, ind2, ind3; 
int AH1, AM1, AH2, AM2, AH3, AM3; 


void setup() 
{
 
 Serial.begin(115200);
 //_____________________________________LCD SETUP_________________________________________________
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Hi, I'm MedStance!");
  lcd.setCursor(3,1);
  lcd.print("Your Personal");
  lcd.setCursor(0,2);
  lcd.print("Medicine Assistance");
  delay(10000);
  lcd.clear();
//_____________________________________BUZZER RTC SETUP_________________________________________________
  pinMode(buz, OUTPUT);
  pinMode(but, INPUT);
   
  Wire.begin(21,22);
  Rtc.Begin();

  SerialPort.begin(115200, SERIAL_8N1, 16, 17);//intializating UART

  //______________________________________Checking the date once____________________________________________
  RtcDateTime now = Rtc.GetDateTime(); //baca waktu dan disimpan dlm object variabel
  pDay = now.Day();

  Serial.print(now.Year());Serial.print("/");Serial.print(now.Month());Serial.print("/");Serial.print(now.Day());Serial.print("-----");
  Serial.print(now.Hour());Serial.print(":");Serial.print(now.Minute());Serial.print(":");Serial.print(now.Second());Serial.println();
  
  if (now.Year() == 2000) {
    Serial.println("RTC not set");
  } else { 
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("TIME CONFIGURATION ");
    lcd.setCursor(6,2);
    lcd.print("SUCCESS ");
    delay(5000);
    successtone(); 
    lcd.clear();
  } 

 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("connecting wifi");
  lcd.setCursor(1,1);
  lcd.print("Connecting to Wi-Fi");
  delay(8000);
  lcd.clear();
  delay(5000);
  
  if (WiFi.status() == WL_CONNECTED){
  Serial.println();
  Serial.print("Connected IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  lcd.setCursor(3,1);
  lcd.print("WiFi Connected ");
  delay(8000);
  lcd.clear();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(2048 , 1024 );
  stream.setBSSLBufferSize(2048, 1024 );
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  delay(2000);

  stream.keepAlive(5, 5, 1);

   if (!Firebase.beginStream(stream_M1, "/schedules/MS1")) {
      Serial.printf("stream M1 begin error, %s\n\n", stream_M1.errorReason().c_str());}
    //stream M2
    if (!Firebase.beginStream(stream_M2, "/schedules/MS2")) {
        Serial.printf("stream M2 begin error, %s\n\n", stream_M2.errorReason().c_str());}
    //stream M3
    if (!Firebase.beginStream(stream_M3, "/schedules/MS3")) {
        Serial.printf("stream M3 begin error, %s\n\n", stream_M3.errorReason().c_str());}


   //STREAM QUANTITAS OBAT
    if (!Firebase.beginStream(stream_qty1, "/meds/1/quantity")) {
    Serial.printf("qty1 begin error, %s\n\n", stream_qty1.errorReason().c_str());}

    if (!Firebase.beginStream(stream_qty2, "/meds/2/quantity")) {
      Serial.printf("qty2 begin error, %s\n\n", stream_qty2.errorReason().c_str());}

    if (!Firebase.beginStream(stream_qty3, "/meds/3/quantity")) {
    Serial.printf("qty3 begin error, %s\n\n", stream_qty3.errorReason().c_str());}

  //stream AH1
  if (!Firebase.beginStream(stream_AH1, "/schedules/T/AH1")) {
      Serial.printf("AH1 begin error, %s\n\n", stream_AH1.errorReason().c_str());}
  //stream AH2
  if (!Firebase.beginStream(stream_AH2, "/schedules/T/AH2")) {
      Serial.printf("AH2 begin error, %s\n\n", stream_AH2.errorReason().c_str());}
  //stream AH3
  if (!Firebase.beginStream(stream_AH3, "/schedules/T/AH3")) {
      Serial.printf("AH3 begin error, %s\n\n", stream_AH3.errorReason().c_str());}
  //stream AM1
  if (!Firebase.beginStream(stream_AM1, "/schedules/T/AM1")) {
      Serial.printf("AM1 begin error, %s\n\n", stream_AM1.errorReason().c_str());}
  //stream AM2
  if (!Firebase.beginStream(stream_AM2, "/schedules/T/AM2")) {
      Serial.printf("AM2 begin error, %s\n\n", stream_AM2.errorReason().c_str());}
  //stream AM3
  if (!Firebase.beginStream(stream_AM3, "/schedules/T/AM3")) {
      Serial.printf("AM3 begin error, %s\n\n", stream_AM3.errorReason().c_str());}

  //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
  config.timeout.socketConnection = 30 * 1000;
  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;
 }// have network
}//setup

void loop() 
{
        if (Firebase.ready() && WiFi.status() == WL_CONNECTED) 
          { 

//______________________________________________MEDICINE SET 1__________________________________________________________________________________________
              if(!Firebase.readStream(stream_M1)){
                Serial.printf("stream  M1 read error, %s\n\n", stream.errorReason().c_str());
              } 
              if(stream_M1.streamAvailable()){
                if(stream_M1.dataType() == "string"){
                  k = stream_M1.to<String>();
                  ind1 = k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS1[0]=M1; 
                  ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS1[1]=M2; 
                  ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS1[2]=M3; 
                }
              }
//______________________________________________MEDICINE SET 2__________________________________________________________________________________________
              if(!Firebase.readStream(stream_M2)){
                Serial.printf("stream  M2 read error, %s\n\n", stream.errorReason().c_str());
              } 
              if(stream_M2.streamAvailable()){
                if(stream_M2.dataType() == "string"){
                  k = stream_M2.to<String>();
                  ind1 = k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS2[0]=M1; 
                  ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS2[1]=M2; 
                  ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS2[2]=M3; 
                }
              }

//______________________________________________MEDICINE SET 3__________________________________________________________________________________________
                if(!Firebase.readStream(stream_M3)){
                  Serial.printf("stream  M3 read error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_M3.streamAvailable()){
                  if(stream_M3.dataType() == "string"){
                   k = stream_M3.to<String>();
                  ind1= k.indexOf(',');           m1 = k.substring(1, ind1);      M1 = m1.toInt(); MS3[0]=M1; 
                  ind2 = k.indexOf(',', ind1+1 );  m2 = k.substring(ind1+1, ind2); M2 = m2.toInt(); MS3[1]=M2; 
                  ind3 = k.indexOf(']', ind2+1 );  m3 = k.substring(ind2+1, ind3); M3 = m3.toInt(); MS3[2]=M3; 
                  }
                }

//_________________________________________________QUANTITY_______________________________________________________
              if(!Firebase.readStream(stream_qty1)){
                Serial.printf("qty1 error, %s\n\n", stream_qty1.errorReason().c_str());
              } 
              if(stream_qty1.streamAvailable()){
                if(stream_qty1.dataType() == "int"){
                  Q1 = stream_qty1.to<int>(); 
                }
              }

              if(!Firebase.readStream(stream_qty2)){
                Serial.printf("qty2 error, %s\n\n", stream_qty2.errorReason().c_str());
              } 
              if(stream_qty2.streamAvailable()){
                if(stream_qty2.dataType() == "int"){
                  Q2 = stream_qty2.to<int>();
                }
              }

             if(!Firebase.readStream(stream_qty3)){
                Serial.printf("qty3 error, %s\n\n", stream_qty3.errorReason().c_str());
              } 
              if(stream_qty3.streamAvailable()){
                if(stream_qty3.dataType() == "int"){
                  Q3 = stream_qty3.to<int>();
                }
              }
//_______________________________________________AH1 DAN AM 1_________________________________________________________
                if(!Firebase.readStream(stream_AH1)){
                  Serial.printf("AH1 error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AH1.streamAvailable()){
                  if(stream_AH1.dataType() == "string"){
                    iv = stream_AH1.to<String>();
                    AH1 = iv.toInt(); 
                  }
                }

                if(!Firebase.readStream(stream_AM1)){
                  Serial.printf("AM1 error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AM1.streamAvailable()){
                  if(stream_AM1.dataType() == "string"){
                    iv = stream_AM1.to<String>();
                    AM1 = iv.toInt(); 
                  }
                }
//_______________________________________________AH2 DAN AM 2_________________________________________________________
                if(!Firebase.readStream(stream_AH2)){
                  Serial.printf("AH2 error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AH2.streamAvailable()){
                  if(stream_AH2.dataType() == "string"){
                    iv = stream_AH2.to<String>();
                    AH2 = iv.toInt();  
                  }
                }

                if(!Firebase.readStream(stream_AM2)){
                  Serial.printf("AM2 error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AM2.streamAvailable()){
                  if(stream_AM2.dataType() == "string"){
                    iv = stream_AM2.to<String>();
                    AM2 = iv.toInt();  
                  }
                }


//_______________________________________________AH3 DAN AM 3_________________________________________________________
                if(!Firebase.readStream(stream_AH3)){
                  Serial.printf("AH3 error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AH3.streamAvailable()){
                  if(stream_AH3.dataType() == "string"){
                    iv = stream_AH3.to<String>();
                    AH3 = iv.toInt(); 
                  }
                }

                if(!Firebase.readStream(stream_AM3)){
                  Serial.printf("AM3 error, %s\n\n", stream.errorReason().c_str());
                } 
                if(stream_AM3.streamAvailable()){
                  if(stream_AM3.dataType() == "string"){
                    iv = stream_AM3.to<String>();
                    AM3 = iv.toInt(); 
                  }
                }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Alarm Pagi : ");
      if (AH1 < 10) lcd.print("0");
      lcd.print(AH1);
      lcd.print(":");
      if (AM1 < 10) lcd.print("0");
      lcd.print(AM1);

      lcd.setCursor(0, 1);
      lcd.print("Alarm Siang: ");
      if (AH2 < 10) lcd.print("0");
      lcd.print(AH2);
      lcd.print(":");
      if (AM2 < 10) lcd.print("0");
      lcd.print(AM2);

      lcd.setCursor(0, 2);
      lcd.print("Alarm Malam: ");
      if (AH3 < 10) lcd.print("0");
      lcd.print(AH3);
      lcd.print(":");
      if (AM3 < 10) lcd.print("0");
      lcd.print(AM3);

      lcd.setCursor(1,3);lcd.print("m1:");lcd.print(Q1);
      lcd.setCursor(7,3);lcd.print("m2:");lcd.print(Q2);
      lcd.setCursor(13,3);lcd.print("m3:");lcd.print(Q3);
    }//fb read
//_______________________________________no network_________________________________________
        else{
            lcd.setCursor(1,0);
            lcd.print("No Network Detected ");
            delay(5000);
            lcd.clear();
            }

        btn = digitalRead(but); 

        Serial.print("MS1 - "); Serial.print(MS1[0]); Serial.print(" - "); Serial.print(MS1[1]); Serial.print(" - "); Serial.println(MS1[2]);
        Serial.print("MS2 - "); Serial.print(MS2[0]); Serial.print(" - "); Serial.print(MS2[1]); Serial.print(" - "); Serial.println(MS2[2]);
        Serial.print("MS3 - "); Serial.print(MS3[0]); Serial.print(" - "); Serial.print(MS3[1]); Serial.print(" - "); Serial.println(MS3[2]);

        Serial.print("btn =");
        Serial.println(btn);
        Serial.print("A1 =");
        Serial.println(a1);
        Serial.print("A2 =");
        Serial.println(a2);
        Serial.print("A3 =");
        Serial.println(a3);
        delay(3000);
          
        RtcDateTime now = Rtc.GetDateTime();    //get the time from the RTC
      
        Years= now.Year(); Months= now.Month(); Days= now.Day();
        Hours= now.Hour(); Min= now.Minute(); Sec= now.Second();

        waktu = String(now.Day()) + "-" + String(now.Month()) + "-" + String(now.Year()) + " " +
                    String(now.Hour()) + ":" + String(now.Minute()) + ":" + String(now.Second());

          if (Hours == AH1 && Min == AM1 && AV1 == 0) // Morning alarm1
          {
            if (btn == 0 && a1 == 0) { // Ketika tombol ditekan untuk pertama kali
                Serial.println("buz high");
                buzOn = true; // Toggle status bunyi buzzer
                a1 = 1; // Setel tombol ke status ditekan
      
              } else if (btn == 1 && a1 == 1 && !serialport1) { // Ketika tombol dilepas setelah ditekan
                  catatTimestamp(fbdo, waktu);
                  SerialPort.print(1);//mengirim trigger utk motor 
                  Serial.println("buz low");
                  a1 = 1;
                  buzOn = false; 
                  noTone(buz);
                  lcd.clear(); 
                  serialport1 = true;
              }

             // AV1 = 1; //dicatat ketika mekanikal udh jalan
          }
          
        else if (Hours == AH2 && Min == AM2 && AV2 == 0) //Day Alarm
          { 
            if (btn == 0 && a2 == 0) { // Ketika tombol ditekan untuk pertama kali
                Serial.println("buz high");
                buzOn = true; // Toggle status bunyi buzzer
                a2 = 1; // Setel tombol ke status ditekan
      
              } else if (btn == 1 && a2 == 1 && !serialport2) { // Ketika tombol dilepas setelah ditekan
                  catatTimestamp(fbdo, waktu);
                  SerialPort.print(2);//mengirim trigger utk motor 
                  Serial.println("buz low");
                  a2 = 1;
                  buzOn = false; 
                  noTone(buz);
                  lcd.clear(); 
                  serialport2 = true;
              }
             // AV2 = 1; //dicatat ketika mekanikal udh jalan
          }

        else if (Hours == AH3 && Min == AM3 && AV3 == 0  ) //Night Alarm 
          {
            if (btn == 0 && a3 == 0) { // Ketika tombol ditekan untuk pertama kali
                Serial.println("buz high");
                buzOn = true; // Toggle status bunyi buzzer
                a3 = 1; // Setel tombol ke status ditekan
      
              } else if (btn == 1 && a3 == 1 && !serialport3) { // Ketika tombol dilepas setelah ditekan
                  catatTimestamp(fbdo, waktu);
                  SerialPort.print(3);//mengirim trigger utk motor 
                  Serial.println("buz high");
                  a3 = 1;
                  buzOn = false; 
                  noTone(buz);
                  lcd.clear(); 
                  serialport3 = true;
              }
          }
    if (buzOn) {
                tone(buz, 3000); 
            } 

    delay(1000);

   if(Days != pDay)
   {
     AV1=0; AV2=0; AV3=0;
     pDay=Days; 
     a1 = 0;
     a2 = 0;
     a3 = 0;
     serialport1 = false;
     serialport2 = false;
     serialport3 = false; 
   }
}

void successtone() { // bunyi yg nandain kalo konfigurasi RTC OK
  tone(buz, 400, 100); // Play first "teng"
  delay(200); // Pause between "teng" sounds
  tone(buz, 600, 100); // Play second "teng"
  delay(200); // Pause after second "teng"
  noTone(buz); // Turn off buzzer
}

void catatTimestamp(FirebaseData &fbdo, const String &waktu){
    int jmlh;
    if (Firebase.getInt(fbdo, "/timestamp/waktuminumobat/count")) {
        jmlh = fbdo.intData(); // Retrieve the integer value
         // Increment the count value by 1
        jmlh++;
    } else {
      // If reading the count value fails, handle the error
      Serial.println("jmlh gagal : " + fbdo.errorReason());
    }

    // Use the incremented count value for your timestampPath
    timePath = "/timestamp/waktuminumobat/" + String(jmlh);
                  
    // Simpan timestamp di Firebase
     if (Firebase.setString(fbdo, timePath, waktu)) {
       Serial.println("waktu obat tercatat: " + waktu);
       // Update count di Firebase
       Firebase.setInt(fbdo, "/timestamp/waktuminumobat/count", jmlh);
    } else {
       Serial.println("waktu gagal tercatat: " + fbdo.errorReason());
    }
}