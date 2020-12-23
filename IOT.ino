#include <ESP8266HTTPClient.h> 
#include <Wire.h>                 
#include <LiquidCrystal_I2C.h>    
LiquidCrystal_I2C lcd(0x27,16,2);
#include "json_parser.h"
#include "WifiConnect.h"

#define s2ms(second) (second*1000)
unsigned long long prev_millis(0);

#define country_code "Turkey"

const char* ssid = "**";              //WIFI SSID Name                             
const char* password = "**";        //WIFI Password

WiFiClient client;      // WiFi client and http client                                                     
HTTPClient http; 

int interval = s2ms(60);
unsigned long long PreviousMillis = 0;
unsigned long long CurrentMillis = interval;
bool bFirstKickMillis = false;

extern bool bGotIpFlag;

static String build_url_from_country(String country)
{
  String url = "http://coronavirus-19-api.herokuapp.com/countries/";
  url = url + country;
  return url;
}

void setup(void)
{ 
  lcd.init();       
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Covid-19 Cases");
   lcd.setCursor(0, 1);
  lcd.print("in Turkey");

  #if defined JSON_DEBUG
    Serial.begin(9600);
  #endif
  
  JSON_LOG("Connecting...");

    WiFi.disconnect();                                                             
    delay(1000);                                                                  
    WiFi.begin(ssid, password);                                                   
    Serial.println("Connected to the WiFi network");                                          
    Serial.println(WiFi.localIP());
}

void loop()
{
  if(bGotIpFlag) bGotIp();
  
  if(bFirstKickMillis) CurrentMillis = millis();
  
  if (!bGotIpFlag && CurrentMillis - PreviousMillis >= interval) 
  {
    if(!bFirstKickMillis) CurrentMillis = 0;
    
    bFirstKickMillis = true;
    
    PreviousMillis = CurrentMillis;
    
    HTTPClient http; 
    http.begin(build_url_from_country(country_code));
    
    int httpCode = http.GET(); 
  
    if(httpCode > 0) 
    {
      String payload = http.getString();
       
      char* JsonArray = (char *)malloc(payload.length() + 1);
      if (!JsonArray) JSON_LOG("ups");
      
      payload.toCharArray(JsonArray, payload.length() + 1);
      
      JSON_LOG(JsonArray);
      
      if (json_validate(JsonArray))
      {
        int confirmed = (int)get_json_value(JsonArray, "cases", INT);
        int deaths = (int)get_json_value(JsonArray, "deaths", INT);
        int recovered = (int)get_json_value(JsonArray, "recovered", INT);
      
        JSON_LOG(confirmed);
        JSON_LOG(deaths);
        JSON_LOG(recovered);
        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Confirmed :");
        lcd.setCursor(0,1);
        lcd.print(confirmed);

        delay(3000);
        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Deaths :");
        lcd.setCursor(0,1);
        lcd.print(deaths);

        delay(3000);

        lcd.setCursor(0,0);
        lcd.print("Recovered :");
        lcd.setCursor(0,1);
        lcd.print(recovered);

        delay(3000);
      }
      
      free(JsonArray);
    }
    
    http.end(); 
  }
}
