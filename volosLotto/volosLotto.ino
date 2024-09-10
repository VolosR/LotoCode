#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include "ui.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <WiFiManager.h>
#include "EEPROM.h"

//EEPROM adresses
/*
0- score
10-17 my numbers
20-games
30-65 how many times
*/

LilyGo_Class amoled;

// WebSocket server URL and port
const char* websocket_server = "chip-nonstop-yam.glitch.me";
const uint16_t websocket_port = 443;
const char* websocket_path = "/"; // Assuming root path for WebSocket

WebSocketsClient webSocket;
lv_obj_t* panels[7];
lv_obj_t* panelsChose[49];
bool selectedNumber[49]={0};
int nOfSel=0;

//variables
int clientCount=0;
int chosenNumber[7]={0};
int myNumbers[7]={12,24,30,8,42,21,5};
String timeStr="";
bool isCon=false;
int foundNumbers=0;
unsigned long score=0;
unsigned long games=0;
unsigned long prizes[8]={0,0,1,10,52,134,620,3860};
long howManyTimes[8]={0};
int slider=0;
bool firstDraw=0;

void saveNumbers(lv_event_t * e)
{
  int position=0;
  for(int i=1;i<49;i++)
  {
    if(selectedNumber[i]==1)
    {
      myNumbers[position]=i;
      position++;
    }
  }

     for(int i=10;i<17;i++)
        EEPROM.writeByte(i, myNumbers[i-10]); 
        EEPROM.commit();
}

void resetAll(lv_event_t * e)
{
  nOfSel=0;
  for(int i=1;i<49;i++)
  {selectedNumber[i]=0;
  lv_obj_set_style_bg_color(panelsChose[i], lv_color_hex(0x1B5187), LV_PART_MAIN);
  }
}

void choseNumber(lv_event_t * e)
{
  
  lv_obj_t * targer = lv_event_get_target(e); 
  for(int i=1;i<49;i++)
  if(targer==panelsChose[i])
 {
  bool didJob=0;
  Serial.println(i);
  if(selectedNumber[i]==1 && didJob==0)
     {
      nOfSel--;
      selectedNumber[i]=0;
      didJob=1;
     }

   if(selectedNumber[i]==0 && nOfSel<7 && didJob==0)
     {
      nOfSel++;
      selectedNumber[i]=1;
      didJob=1;
     }

  if(selectedNumber[i]==1)
   lv_obj_set_style_bg_color(panelsChose[i], lv_color_hex(0xF141F5), LV_PART_MAIN);
  if(selectedNumber[i]==0)
   lv_obj_set_style_bg_color(panelsChose[i], lv_color_hex(0x1B5187), LV_PART_MAIN);
 }
 
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("Disconnected from WebSocket server");
            break;
        case WStype_CONNECTED:
            Serial.println("Connected to WebSocket server");
            _ui_label_set_property(ui_initMsg, _UI_LABEL_PROPERTY_TEXT, "Connected to WebSocket server,waiting for numbers!");
            break;
        case WStype_TEXT: {
            Serial.printf("Message from server: %s\n", payload);
            if(firstDraw){
            foundNumbers=0;
            StaticJsonDocument<500> doc;
            DeserializationError error = deserializeJson(doc, payload);
              if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
             JsonArray numbers = doc["numbers"];

             for(int i=0;i<7;i++)
             lv_obj_set_style_bg_color(panels[i], lv_color_hex(0x14BFD0), LV_PART_MAIN);

            // Iteracija kroz niz i ispisivanje svakog broja
            for (int i = 0; i < numbers.size(); i++) {
                chosenNumber[i] = numbers[i];
                Serial.print("Number ");
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.println(chosenNumber[i]);
                
                for(int j=0;j<7;j++)
                {
                  if(chosenNumber[i]==myNumbers[j])
                  {
                    lv_obj_set_style_bg_color(panels[i], lv_color_hex(0xF141F5), LV_PART_MAIN);
                    Serial.println("Found");
                    foundNumbers++;
                  }
                }
              }
               score=score+prizes[foundNumbers];
               
               howManyTimes[foundNumbers]++;

               _ui_label_set_property(ui_numWinings, _UI_LABEL_PROPERTY_TEXT, String(howManyTimes[2]).c_str());
               _ui_label_set_property(ui_numWinings1, _UI_LABEL_PROPERTY_TEXT, String(howManyTimes[3]).c_str());
               _ui_label_set_property(ui_numWinings2, _UI_LABEL_PROPERTY_TEXT, String(howManyTimes[4]).c_str());
               _ui_label_set_property(ui_numWinings3, _UI_LABEL_PROPERTY_TEXT, String(howManyTimes[5]).c_str());
               _ui_label_set_property(ui_numWinings4, _UI_LABEL_PROPERTY_TEXT, String(howManyTimes[6]).c_str());
               _ui_label_set_property(ui_numWinings5, _UI_LABEL_PROPERTY_TEXT, String(howManyTimes[7]).c_str());


              _ui_label_set_property(ui_number, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[0]).c_str());
              _ui_label_set_property(ui_number1, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[1]).c_str());
              _ui_label_set_property(ui_number2, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[2]).c_str());
              _ui_label_set_property(ui_number3, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[3]).c_str());
              _ui_label_set_property(ui_number4, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[4]).c_str());
              _ui_label_set_property(ui_number5, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[5]).c_str());
              _ui_label_set_property(ui_number6, _UI_LABEL_PROPERTY_TEXT, String(chosenNumber[6]).c_str());
              _ui_label_set_property(ui_scoreLbl, _UI_LABEL_PROPERTY_TEXT, String(score).c_str());

              // Dobijanje broja klijenata
               clientCount = doc["clientCount"];
               Serial.print("Client Count: ");
               Serial.println(clientCount);
               _ui_label_set_property(ui_noDevicesLbl, _UI_LABEL_PROPERTY_TEXT, String(clientCount).c_str());

              // Dobijanje vremena
               const char* time = doc["time"];
               const char* msg=doc["message"];
              Serial.print("Time: ");
              Serial.println(time);
              timeStr=String(time).substring(11,19);
              timeStr="UTC: "+timeStr;
              games++;
              _ui_label_set_property(ui_Label14, _UI_LABEL_PROPERTY_TEXT, timeStr.c_str());
              _ui_label_set_property(ui_msg, _UI_LABEL_PROPERTY_TEXT, msg);
              _ui_label_set_property(ui_totalGames, _UI_LABEL_PROPERTY_TEXT, String(games).c_str());
               slider=-1;

              EEPROM.writeULong(0, score);
              EEPROM.writeULong(20, games);
               for(int i=0;i<8;i++)
              EEPROM.writeLong(30+(i*5),howManyTimes[i]);
              EEPROM.commit();
              lv_obj_add_flag(ui_InitPanel, LV_OBJ_FLAG_HIDDEN);
            }
            if(firstDraw==0)
            firstDraw=1;
            break;
        }
        case WStype_BIN:
            Serial.println("Binary message received");
            break;
        case WStype_PING:
            Serial.println("Ping received");
            break;
        case WStype_PONG:
            Serial.println("Pong received");
            break;
        case WStype_ERROR:
            Serial.println("WebSocket Error");
            break;
    }
}



void setup() {


      Serial.begin(115200);
      Serial.println("lets start");
      EEPROM.begin(500);
      
      int chck=EEPROM.readByte(490);
      int chck2=EEPROM.readByte(480);

      //if board never used it will reset eprom to 0
      if(chck!=13 && chck2!=97)
      {
        for(int i=0;i<100;i++)
        EEPROM.writeByte(i, 0); 

        EEPROM.writeByte(490, 13);
        EEPROM.writeByte(480, 97);

        for(int i=10;i<17;i++)
        EEPROM.writeByte(i, random(0,47)); 

        EEPROM.commit();
        Serial.println("eeprom reseted");
      }

      //reading values from EEPROM

      //reading myNumbers
      for(int i=10;i<17;i++)
      myNumbers[i-10]=EEPROM.readByte(i); 

       for(int i=0;i<8;i++)
       howManyTimes[i]=EEPROM.readLong(30+(i*5));


      // reading score
      score=EEPROM.readULong(0);
      games=EEPROM.readULong(20);
      bool rslt = false;
      rslt = amoled.begin();

    if (!rslt) {
        while (1) {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }
    amoled.setBrightness(140);
    beginLvglHelper(amoled);
    ui_init();

    panels[0] = ui_numberPanel;
    panels[1] = ui_numberPanel1;
    panels[2] = ui_numberPanel2;
    panels[3] = ui_numberPanel3;
    panels[4] = ui_numberPanel4;
    panels[5] = ui_numberPanel5;
    panels[6] = ui_numberPanel6;

    panelsChose[1]=ui_numPan1;
    panelsChose[2]=ui_numPan2;
    panelsChose[3]=ui_numPan3;
    panelsChose[4]=ui_numPan4;
    panelsChose[5]=ui_numPan5;
    panelsChose[6]=ui_numPan6;
    panelsChose[7]=ui_numPan7;
    panelsChose[8]=ui_numPan8;
    panelsChose[9]=ui_numPan9;
    panelsChose[10]=ui_numPan10;
    panelsChose[11]=ui_numPan11;
    panelsChose[12]=ui_numPan12;
    panelsChose[13]=ui_numPan13;
    panelsChose[14]=ui_numPan14;
    panelsChose[15]=ui_numPan15;
    panelsChose[16]=ui_numPan16;
    panelsChose[17]=ui_numPan17;
    panelsChose[18]=ui_numPan18;
    panelsChose[19]=ui_numPan19;
    panelsChose[20]=ui_numPan20;
    panelsChose[21]=ui_numPan21;
    panelsChose[22]=ui_numPan22;
    panelsChose[23]=ui_numPan23;
    panelsChose[24]=ui_numPan24;
    panelsChose[25]=ui_numPan25;
    panelsChose[26]=ui_numPan26;
    panelsChose[27]=ui_numPan27;
    panelsChose[28]=ui_numPan28;
    panelsChose[29]=ui_numPan29;
    panelsChose[30]=ui_numPan30;
    panelsChose[31]=ui_numPan31;
    panelsChose[32]=ui_numPan32;

    panelsChose[33]=ui_numPan33;
    panelsChose[34]=ui_numPan34;
    panelsChose[35]=ui_numPan35;
    panelsChose[36]=ui_numPan36;
    panelsChose[37]=ui_numPan37;
    panelsChose[38]=ui_numPan38;
    panelsChose[39]=ui_numPan39;
    panelsChose[40]=ui_numPan40;
    panelsChose[41]=ui_numPan41;
    panelsChose[42]=ui_numPan42;
    panelsChose[43]=ui_numPan43;
    panelsChose[44]=ui_numPan44;
    panelsChose[45]=ui_numPan45;
    panelsChose[46]=ui_numPan46;
    panelsChose[47]=ui_numPan47;
    panelsChose[48]=ui_numPan48;

  
   
  Serial.println(EEPROM.readLong(0));
  
}

void loop() {
  lv_task_handler();
  delay(5);

  if(slider<2400)
  slider++;
  lv_slider_set_value(ui_Slider1, slider, LV_ANIM_OFF);


   if(!isCon)
 {
    WiFiManager wifiManager;
    
    // Povećaj timeout na npr. 5 minuta (300 sekundi)
    wifiManager.setConfigPortalTimeout(5000);

    if(!wifiManager.autoConnect("VolosWifiConf","password")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }

    Serial.println("Connected.");
    _ui_label_set_property(ui_initMsg, _UI_LABEL_PROPERTY_TEXT, "Connected to WiFi! Connecting to server!!");
    isCon=true;
    // Initialize WebSocket
    webSocket.beginSSL(websocket_server, websocket_port, websocket_path);
    
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000); // 5 seconds
    // To avoid blocking setup
 }

  _ui_label_set_property(ui_myNumber1, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[0]).c_str());
  _ui_label_set_property(ui_myNumber2, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[1]).c_str());
  _ui_label_set_property(ui_myNumber3, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[2]).c_str());
  _ui_label_set_property(ui_myNumber4, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[3]).c_str());
  _ui_label_set_property(ui_myNumber5, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[4]).c_str());
  _ui_label_set_property(ui_myNumber6, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[5]).c_str());
  _ui_label_set_property(ui_myNumber7, _UI_LABEL_PROPERTY_TEXT, String(myNumbers[6]).c_str());

  webSocket.loop();
  
}