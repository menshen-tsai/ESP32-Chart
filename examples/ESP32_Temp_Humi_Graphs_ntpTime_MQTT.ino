// (C) D Bird 2016
// DHT22 temperature and humidity display using graphs
//
// https://github.com/G6EJD/ESP8266-TN004.git


#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "ChartGraph.h"
#include <WiFi.h>
#include <Ticker.h>
#include <Time.h>
#include <MQTT.h>
#include "config.h"

#define ONBOARDLED 2 // Built in LED on ESP32-T
#define _cs   22  // goes to TFT CS
#define _dc   17  // goes to TFT DC
#define _mosi 23  // goes to TFT MOSI
#define _sclk 18  // goes to TFT SCK/CLK
#define _rst  16  // goes to TFT RESET
#define _miso  19 // goes to TFT MISO, Not connected


#define BACKGROUND  ILI9341_BLUE
#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  false


float humidity, temperature=0.0;  // Values read from sensor


const char* NTP_SERVER = "tw.pool.ntp.org";
const char* TZ_INFO    = "CST-8";
const char* MQTT_BROKER = "192.168.0.195";
MQTTClient mqttClient;
WiFiClient net;

time_t this_second = 0;
time_t last_second = 0;

// Use hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);
//// void           ChartGraph(&tft, x_pos, y_pos, width, height, Y1Min, Y1Max, title,         auto_scale,    barchart_mode, colour) 
ChartGraph chart1 = ChartGraph(&tft, 5,   20,    200,   80,     0,     50,    "Temperature", autoscale_on,  barchart_off,  ILI9341_RED) ;
ChartGraph chart2 = ChartGraph(&tft, 5,   140,   200,   80,     0,     100,   "Humidity",    autoscale_off, barchart_on,   ILI9341_GREEN);


//----------------------------------------------------------------------------------------------------
 /* (C) D L BIRD
 *  This function will draw a graph on a TFT / LCD display, it requires an array that contrains the data to be graphed.
 *  The variable 'max_readings' determines the maximum number of data elements for each array. Call it with the following parametric data:
 *  x_pos - the x axis top-left position of the graph
 *  y_pos - the y-axis top-left position of the graph, e.g. 100, 200 would draw the graph 100 pixels along and 200 pixels down from the top-left of the screen
 *  width - the width of the graph in pixels
 *  height - height of the graph in pixels
 *  Y1_Max - sets the scale of plotted data, for example 5000 would scale all data to a Y-axis of 5000 maximum
 *  data_array1 is parsed by value, externally they can be called anything else, e.g. within the routine it is called data_array1, but externally could be temperature_readings
 *  auto_scale - a logical value (TRUE or FALSE) that switches the Y-axis autoscale On or Off
 *  barchart_on - a logical value (TRUE or FALSE) that switches the drawing mode between barhcart and line graph
 *  barchart_colour - a sets the title and graph plotting colour
 *  If called with Y!_Max value of 500 and the data never goes above 500, then autoscale will retain a 0-500 Y scale, if on, it will increase the scale to match the data to be displayed, and reduce it accordingly if required.
 *  auto_scale_major_tick, set to 1000 and autoscale with increment the scale in 1000 steps.
 */

void connect() {
  uint8_t count = 30;
  while ((WiFi.status() != WL_CONNECTED) && (count > 0)){
    Serial.print(".");
    count--;
    delay(1000);
  }
  if (count == 0) {
    Serial.println("\nConnecting to WiFi AP failed, Restarting...");
    ESP.restart();
  }
  Serial.print("\nWifi Connected, ");
  Serial.print("connecting to MQTT Broker ...");
  while (!mqttClient.connect(MQTT_BROKER, "try0", "try0")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMQTT Broker connected!");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (topic.equals("outTopic/temperature")) {
    temperature = payload.toFloat();
  }  
  // else if (topic.equals("outTopic/pressure")) {
  // pressure = payload.toFloat();
  // }
}

 
void setup() {
  time_t t0;
  char buf[30], ipBuf[22];
  char unit[] = {char(247), 'C', 0};

  uint8_t count = 30;
    
  Serial.begin(115200);
  while (!Serial && (millis() <= 1000));
  pinMode (ONBOARDLED, OUTPUT); // Onboard LED
  digitalWrite (ONBOARDLED, HIGH); // Switch off LED
  Serial.println();
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(" compiled at ");
  Serial.print(__DATE__);Serial.print(" ");Serial.println(__TIME__);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND);  
  updateStatus("Connecting to WiFi AP");
  WiFi.mode (WIFI_STA);
  WiFi.begin (WIFI_SSID, WIFI_PASS);

  while ((WiFi.status() != WL_CONNECTED) && (count > 0)){
        delay(500);
        Serial.print(".");
        count--;
  }
  if (count == 0) {
    Serial.println("\nConnecting to WiFi AP failed, Restarting...");
    ESP.restart();
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  String ip = WiFi.localIP().toString();
  ip.toCharArray(ipBuf, 22);
  sprintf(buf, "IP: %s       ", ipBuf);
  updateStatus(buf);

  configTzTime(TZ_INFO, NTP_SERVER);
  time(&t0);
  Serial.printf("Connecting to NTP server  ");
  count = 30;
  while((t0 < 1000) && (count >0)) {
    count--;
    time(&t0);
    Serial.print(":");
    delay(500);
  }
  if (count >0) {
    Serial.println(ctime(&t0));
  } else {
    Serial.println("\nNTP Server connection failed");
  }
  tft.setTextColor(ILI9341_WHITE, BACKGROUND);
  chart1.BackgroundColor(BACKGROUND) ;
  chart2.BackgroundColor(BACKGROUND) ; 
  chart1.setUnit(unit);
  chart2.setUnit("%");
  chart1.begin();
  chart2.begin();

// Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  mqttClient.begin("192.168.0.195", net);
  mqttClient.onMessage(messageReceived);
  Serial.print("\nconnecting to MQTT Broker ...");
  while (!mqttClient.connect("192.168.0.195", "try0", "try0")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");  
  mqttClient.subscribe("#");
  // client.unsubscribe("/hello");
}

void loop() {
  struct tm *timeInfo;
//  char buf[20];
  float t, humidity;

  mqttClient.loop();
  
  if (!mqttClient.connected()) {
    connect();
  }
  time(&this_second); // until time is set, it remains 0
  if (this_second != last_second)
  {
    last_second = this_second;
    timeInfo = localtime(&this_second);
    updateTime(timeInfo);
//  if ((this_second % 60) == 0) {
    get_temperature_and_humidity(&t, &humidity);

    chart1.addData(temperature);
////    chart1.addData(t);
    chart2.addData(humidity);    
    chart1.UpdateGraph();
    chart2.UpdateGraph();
//  }
  }
  delay(10); 
}  

void get_temperature_and_humidity(float *t, float *h) {
//  temperature = dht.readTemperature() + temp_error_offset; // Read temperature as C
//  humidity    = dht.readHumidity()    + humi_error_offset; // Read humidity as %rh
  static float i;
  *t = 25.0 + 5.0*sin(i);
  i=i+0.1;
  *h = 50.0 + 25.0*cos(i);
  // Check if any reads failed and exit early (to try again).
  if (isnan(*t) || isnan(*h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void updateStatus(char * s) {
  tft.drawRect(0,230,320,10,BACKGROUND);  
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW, BACKGROUND);
  tft.setCursor(0, 230);
  tft.print(s);
}

void updateTime(struct tm* timeInfo) {
  char buf[20];
  
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW, BACKGROUND);
  tft.setCursor(0,0);
  strftime(buf, 20, "%b/%d/%Y  ", timeInfo);
  tft.print(buf);
  tft.setCursor(0,10);
  strftime(buf, 20, "%T", timeInfo);
  tft.print(buf);
}

