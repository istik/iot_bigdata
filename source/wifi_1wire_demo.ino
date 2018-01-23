
/*************************************************************************************************
 * 
 ************************************************************************************************/
#define SHOW_DALLAS_ERROR        // uncomment to show Dallas ( CRC ) errors on Serial.
#define ONEWIRE_PIN           25    // OneWire Dallas sensors are connected to this pin
#define MAX_NUMBER_OF_SENSORS 1    // maximum number of Dallas sensors
#include "OneWire.h" 
#include <WiFi.h>
#include <AWS_IOT.h>


/* Time Stamp */
#include <NTPClient.h>
#include <WiFiUdp.h>

#define NTP_OFFSET  1  * 60 * 60 // In seconds, Convert UTC to Paris Time
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.pool.ntp.org"
/* Time Stamp End*/

/* LED*/
#include <SPI.h>
#include "LedMatrix.h"
#define NUMBER_OF_DEVICES 4
#define CS_PIN 15
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);


AWS_IOT hornbill;


const char* ssid = "iPhone 7 Linnnnnnn";
const char* password = "1234567890";
char HOST_ADDRESS[]="a1x30lokt6cbmu.iot.eu-west-1.amazonaws.com";
char CLIENT_ID[]= "MyTestDemo";
char TOPIC_NAME[]= "/sbs/devicedata/temperature";

OneWire  ds( ONEWIRE_PIN );        // (a 4.7K pull-up resistor is necessary)


struct sensorStruct
{
  byte addr[8];
  float temp;
  String name;
  String time;
  long epochdatetime;
//  char* test;
} sensor[MAX_NUMBER_OF_SENSORS];

byte numberOfFoundSensors;

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}
void setup()
{
  Serial.begin( 115200 );
  Serial.println( "\n\nMultiple DS18B20 sensors as task ESP32 example." );
  
  Serial.printf( "Connecting to %s with password %s\n", ssid,  password );

  xTaskCreatePinnedToCore(
    tempTask,                       /* Function to implement the task */
    "tempTask ",                    /* Name of the task */
    4000,                           /* Stack size in words */
    NULL,                           /* Task input parameter */
    5,                              /* Priority of the task */
    NULL,                           /* Task handle. */
    1);                             /* Core where the task should run */
    
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
ledMatrix.init();
ledMatrix.setIntensity(5);

  while ( WiFi.status() != WL_CONNECTED )
  {
    vTaskDelay( 250 /portTICK_PERIOD_MS );
    Serial.print( "." );
  }
   timeClient.begin();


  if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if(0==hornbill.subscribe(TOPIC_NAME,mySubCallBackHandler))
        {
            Serial.println("Subscribe Successfull");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }

    delay(2000);
  Serial.println();
}


void loop()
{  
  if ( numberOfFoundSensors )
  {
    Serial.println( String( millis() / 1000.0 ) + " sec" );
    for ( byte thisSensor = 0; thisSensor < numberOfFoundSensors; thisSensor++ )
    {
      Serial.println("Temperature: " + String( sensor[thisSensor].temp / 16.0 ) + "°C, "+ " Epoch: "+sensor[thisSensor].epochdatetime);
      //sprintf(payload,(char[])("Temperature: " + String( sensor[thisSensor].temp / 16.0 ) + "°C, Time: " + sensor[thisSensor].time+ " Epoch: "+sensor[thisSensor].epochdatetime))
      //sprintf(payload,"{\"device\": \"temperature1\", \"temperature\":%f,\"timestamp_t\": \"%s\"}",sensor[thisSensor].temp / 16.0, F("test")); 
      snprintf(payload,512,"{\"device\": \"temperature1\", \"temperature\":%f,\"timestamp_t\": \"%d\"}",sensor[thisSensor].temp / 16.0, sensor[thisSensor].epochdatetime); 

       ledMatrix.setText(String( sensor[thisSensor].temp / 16.0 ));
       ledMatrix.scrollTextLeft();
       ledMatrix.drawText();
       ledMatrix.commit();

       if(hornbill.publish(TOPIC_NAME,payload) == 0)
        {        
          Serial.print("Publish Message:");
          Serial.println(payload);
          }
          else
          {
              Serial.println("Publish failed");
          }
     }
  }
  else
  {
    Serial.println( "No Dallas sensors." );
  }
  vTaskDelay( 5000 / portTICK_PERIOD_MS );
}

void tempTask( void * pvParameters )
{
  numberOfFoundSensors = 0;
  byte currentAddr[8];
  while ( ds.search( currentAddr ) && numberOfFoundSensors < MAX_NUMBER_OF_SENSORS )
  {
    //Serial.write( "Sensor "); Serial.print( counter ); Serial.print( ":" );
    for ( byte i = 0; i < 8; i++)
    {
      //Serial.write(' ');
      //Serial.print( currentAddr[i], HEX );
      sensor[numberOfFoundSensors].addr[i] = currentAddr[i];
    }
    //sensor[numberOfFoundSensors].name = 'T ' + char( numberOfFoundSensors );
    numberOfFoundSensors++;
  }
  Serial.printf( "%i Dallas sensors found.\n", numberOfFoundSensors );

  if ( !numberOfFoundSensors )
  {
    vTaskDelete( NULL );
  }

  /* main temptask loop */

  while (1)
  {
    for ( byte thisSensor = 0; thisSensor < numberOfFoundSensors; thisSensor++)
    {
      ds.reset();
      ds.select( sensor[thisSensor].addr );
      ds.write( 0x44, 0);        // start conversion, with parasite power off at the end
    }

    vTaskDelay( 750 / portTICK_PERIOD_MS); //wait for conversion ready

    for ( byte thisSensor = 0; thisSensor < numberOfFoundSensors; thisSensor++)
    {
      byte data[12];
      ds.reset();
      ds.select( sensor[thisSensor].addr );
      ds.write( 0xBE );         // Read Scratchpad

      //Serial.print( "Sensor " );Serial.print( thisSensor ); Serial.print("  Data = ");
      //Serial.println( present, HEX );
      //Serial.print(" ");
      for ( byte i = 0; i < 9; i++)
      { // we need 9 bytes
        data[i] = ds.read(  );
        //Serial.print(data[i], HEX);
        //Serial.print(" ");
      }
      //Serial.println();

      byte type_s;
      // the first ROM byte indicates which chip
      switch ( sensor[thisSensor].addr[0] )
      {
        case 0x10:
          //Serial.println("  Chip = DS18S20");  // or old DS1820
          type_s = 1;
          break;
        case 0x28:
          //Serial.println("  Chip = DS18B20");
          type_s = 0;
          break;
        case 0x22:
          //Serial.println("  Chip = DS1822");
          type_s = 0;
          break;
        default:
#ifdef SHOW_DALLAS_ERROR
          Serial.println("Device is not a DS18x20 family device.");
#endif
          return;
      }

      int16_t raw;
      if ( OneWire::crc8(data, 8) != data[8])
      {
#ifdef SHOW_DALLAS_ERROR
        // CRC of temperature reading indicates an error, so we print a error message and discard this reading
        Serial.print( millis() / 1000.0 ); Serial.print( " - CRC error from device " ); Serial.println( thisSensor );
#endif
      }
      else
      {
        raw = (data[1] << 8) | data[0];
        if (type_s)
        {
          raw = raw << 3; // 9 bit resolution default
          if (data[7] == 0x10)
          {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
          }
        }
        else
        {
          byte cfg = (data[4] & 0x60);
          // at lower res, the low bits are undefined, so let's zero them
          if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
          else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
          else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
          //// default is 12 bit resolution, 750 ms conversion time
        }
        
        timeClient.update();
        String formattedTime = timeClient.getFormattedTime();
        long formattedDate = timeClient.getEpochTime();
        sensor[thisSensor].temp = raw;
        //sensor[thisSensor].time = formattedTime;
        sensor[thisSensor].epochdatetime = formattedDate;
        // + "\", \"timestamp_t\": \"" + formattedDate + "\"  \"temperature\":"+ raw /16.0 +"}");
        //sprintf((char *)sensor[thisSensor].test,"{\"device\": \"temperature1\"，\"time_t\": \"%s\", \"timestamp_t\": \"%d\"  \"temperature\":%f}",formattedTime,formattedDate,raw / 16.0 );       
    }
  }
  }}
