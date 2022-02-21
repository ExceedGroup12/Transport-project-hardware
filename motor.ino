#include<WiFi.h>
#include<HTTPClient.h>
#include<ArduinoJson.h>
#include <string.h>
#include <stdio.h>


int Lsensor = 4;
int Rsensor = 12;
int LDrive1 = 18;
int LDrive2 = 19;
int RDrive1 = 25; 
int RDrive2 = 26; 
int LPWM = 22;    // PWM left motor
int RPWM = 23;    // PWM right motor
int SPEED = 150; 
int Lch = 0;
int Rch= 1;
int freq = 1000;
int res = 8;
int _move = 0;
int _direction = 0;
int nowdir = 0;
static int a;
char str[50];


const char* ssid="pooh A52s";
const char* password = "Xxxmen_2543";
const char* urlGET = "https://ecourse.cpe.ku.ac.th/exceed12/api/get-status";
const char* urlPOST = "https://ecourse.cpe.ku.ac.th/exceed12/api/reached";


const int _size = 2*JSON_OBJECT_SIZE(1);
StaticJsonDocument<_size> JSONPost;
StaticJsonDocument<_size> JSONGet;

void setup(){
  Serial.begin(9600);
  pinMode(Lsensor, INPUT);
  pinMode(Rsensor, INPUT);
  pinMode(LDrive1, OUTPUT);
  pinMode(LDrive2, OUTPUT);
  pinMode(RDrive1, OUTPUT);
  pinMode(RDrive2, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(RPWM, OUTPUT);
  ledcSetup(Lch, freq,res);
  ledcAttachPin(LPWM, Lch);
  ledcSetup(Rch, freq,res);
  ledcAttachPin(RPWM, Rch);  
  ledcWrite(Lch,SPEED);
  ledcWrite(Rch,SPEED);
  WiFi_Connect();
}

void fd(){
    digitalWrite(LDrive1, HIGH);
    digitalWrite(LDrive2, LOW);
    digitalWrite(RDrive1, HIGH);
    digitalWrite(RDrive2, LOW);
}

void tr(){
    digitalWrite(LDrive1, LOW);
    digitalWrite(LDrive2, LOW);
    digitalWrite(RDrive1, HIGH);
    digitalWrite(RDrive2, LOW);
}

void tl(){
    digitalWrite(LDrive1, HIGH);
    digitalWrite(LDrive2, LOW);
    digitalWrite(RDrive1, LOW);
    digitalWrite(RDrive2, LOW);
}

void stopp(){
    digitalWrite(LDrive1, LOW);
    digitalWrite(LDrive2, LOW);
    digitalWrite(RDrive1, LOW);
    digitalWrite(RDrive2, LOW);
    
}

void turn(){
    ledcWrite(Lch,120);
    ledcWrite(Rch,120);
    digitalWrite(LDrive1, LOW);
    digitalWrite(LDrive2, HIGH);
    digitalWrite(RDrive1, HIGH);
    digitalWrite(RDrive2, LOW);
    delay(1600);
    while(!digitalRead(Rsensor)){
      digitalWrite(LDrive1, LOW);
      digitalWrite(LDrive2, HIGH);
      digitalWrite(RDrive1, HIGH);
      digitalWrite(RDrive2, LOW);
    }
    while(!digitalRead(Lsensor)){
      tl();
    }
    ledcWrite(Lch,SPEED);
    ledcWrite(Rch,SPEED);
    stopp();
}

void WiFi_Connect(){
  WiFi.disconnect();
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
}

void _get(){
    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;
  
      http.begin(urlGET);
  
      int httpCode = http.GET();
  
      if(httpCode == HTTP_CODE_OK){
        String payload = http.getString();
        DeserializationError err = deserializeJson(JSONGet, payload);
        if(err){
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(err.c_str());
        }else{
          Serial.println(httpCode);
          //Serial.print("move : ");
          //Serial.println((int)JSONGet["move"]);
          _move = JSONGet["move"];
        }
      }else{
        Serial.println(httpCode);
        Serial.println("ERROR on HTTP Request");
      }
    }else{
      WiFi_Connect();
    }
}

void _post(int a){
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    http.begin(urlPOST);
    http.addHeader("Content-Type","application/json");
  
    JSONPost["a"] = 1;
    serializeJson(JSONPost,str);
    int httpCode = http.POST(str);

    if(httpCode == HTTP_CODE_OK){
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
    }else{
      Serial.println(httpCode);
      Serial.println("ERROR on HTTP Request");
    }
  }else{
    WiFi_Connect();
  }
}



void loop(){
  while(_move == 0){
    _get();
    delay(1000);
    printf("-----move = %d ----\n",_move);
  }

  fd();
  delay(50);
  
  while(_move > 0){
  if (!(digitalRead(Lsensor)) && !(digitalRead(Rsensor))) {  //เงือนไขเดินหน้า
    fd();
  }

  else if(digitalRead(Lsensor) && !(digitalRead(Rsensor)))   {  //เงือนไขเลี้ยวขวา
    tr();
    
  }

  else if (!(digitalRead(Lsensor)) && digitalRead(Rsensor))  {  //เงือนไขเลี้ยวซ้าย
    //ledcWrite(Lch,SPEED);
    tl();
  }
  
  else if(digitalRead(Lsensor) && digitalRead(Rsensor))   {  //เงือนไขหยุด
    printf("----------------------------------\n");
    _move--;
    fd();
    delay(200);
  }
  printf("%d %d = %d\n",digitalRead(Lsensor),digitalRead(Rsensor) ,_move);
  delay(10);

  }
  stopp();
  _post(1);  
}
