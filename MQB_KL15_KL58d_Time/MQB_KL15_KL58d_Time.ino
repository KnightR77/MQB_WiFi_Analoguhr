#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <Timezone.h>
#include <time.h>
#include <TimeLib.h>
//#include <ArduinoJSON.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <mcp_can.h>
#include <SPI.h>

#define TZ_OFFSET 0
#define NTP_INTERVAL 60000
#define NTP_SVR "0.us.pool.ntp.org"

const char* ssid = "";
const char* passwd = "";
const String days[8] = {"", "Sun","Mon","Tue","Wed","Thu","Fri","Sat"};\
String output = "";
int tmphour = 0;
int tmpmin = 0;
// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

WiFiUDP ntpudp;
NTPClient ntpc(ntpudp, NTP_SVR, TZ_OFFSET, NTP_INTERVAL);

#define KLEMMEN_STATUS_01_ID 0x3C0
#define KLEMMEN_58_ID 0x5F0
#define GW_TIME 0x6B6

unsigned char kStatusBuf[4] = {0x00,0x00,0x03,0x00};
// unsigned char checkSum[16]  = {0x74,0xC1,0x31,0x84,0xFE,0x4B,0xBB,0x0E,0x4F,0xFA,0x0A,0xBF,0xC5,0x70,0x80,0x35};
unsigned char kl58buf[8] = {0xFD, 0xFD, 0xFD, 0x00, 0x00, 0x59, 0x00, 0x00};
unsigned char timebuf[6] = {0x59, 0x15, 0x0B, 0x06, 0x08, 0x1C};
int i=0;

MCP_CAN CAN(5);


void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  // put your setup code here, to run once:
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED){
    Serial.println("Wifi Not Connected");
    delay(1000);
  }
  Serial.println("Wifi Connected !!!");
  delay(1000);
  ntpc.begin();
  ntpc.update();
  setTime(usET.toLocal(ntpc.getEpochTime()));
  Serial.print(hour());
  Serial.print(" ");
  Serial.println(minute());
  CAN.begin(MCP_ANY,CAN_500KBPS,MCP_8MHZ); 
  CAN.setMode(MCP_NORMAL);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (tmphour != hour()){
    ntpc.update();
    setTime(usET.toLocal(ntpc.getEpochTime()));
  }
  tmphour = hour();
  tmpmin = minute();
  // if (tmpmin != minute()){
    // Serial.println("Updating CAN Time");
  // }

  // kStatusBuf[0]=checkSum;
  kStatusBuf[1]=i;
  CAN.sendMsgBuf(KLEMMEN_STATUS_01_ID, 0, 4, kStatusBuf);
  CAN.sendMsgBuf(KLEMMEN_58_ID, 0, 8, kl58buf);
  i++;
  if (i>15) 
  {
    i=0;
    timebuf[3] = (char)hour();
    timebuf[4] = (char)minute();
    CAN.sendMsgBuf(GW_TIME, 0, 6, timebuf);
    };
  delay(100);
}
