/*
This code is an example of controlling the e-ink screen of the 
xiaomi mijia Temperature Humidity Pro 
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
// https://github.com/PaulStoffregen/Time
#include <TimeLib.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
// https://github.com/tzapu/WiFiManager
#include <WiFiManager.h>
#include "XiaomiMijaTempHumPro.h"

// update via NTP once every DO_NTP_UPDATE seconds (typically an hour)
#define DO_NTP_UPDATE 3600

// display
XiaomiMijaTempHumPro my_display;

// wifimanager
WiFiManager wifiManager;
// NTP Server
static const char ntpServerName[] = "nl.pool.ntp.org";
const int timeZone = 1; // Central European Time
// UDP settings
WiFiUDP Udp;
unsigned int localPort = 8888; // local port to listen for UDP packets

// STRUCT TO SAVE VARIABLES
struct
{
    time_t unix_time = 0;
    int secs_since_NTP = DO_NTP_UPDATE; // the number of seconds since the last NTP update
    int valid_time = 0;                 // indicate if the NTP update was unsuccessful
} rtc_data;

// functions to get time from ntp (from https://github.com/PaulStoffregen/Time)

const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

uint32_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0)
        ; // discard any previously received packets
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];

            // 2208988800UL = the 70 years from 1900 to 1970
            rtc_data.unix_time = secsSince1900 - 2208988800UL;
            setTime(rtc_data.unix_time + timeZone * 3600);
            // valid response
            return 1;
        }
    }
    return 0; // return 0 if unable to get the time
}

void do_NTP_update()
{
    // Get the time via NTP

    // start UDP
    Udp.begin(localPort);

    // get the time in seconds since 1970
    uint32_t my_time = getNtpTime();
    if (my_time == 1)
    { // there was a valid response from the NTP server
        rtc_data.valid_time = 1;
        rtc_data.secs_since_NTP = 0;
    }
    else
    { // the NTP server did not respond
        rtc_data.valid_time = 0;
    }
}

// =========================================================== DISPLAY SETTINGS

void update_display()
{ // display the time

    // start building a new screen
    my_display.start_new_screen();

    int h = hour();
    int m = minute();
    int d = day();
    int mo = month();
    int y = year();

    // minutes
    my_display.set_number(m % 10, TIME4);
    my_display.set_number(m / 10, TIME3);
    // hours
    my_display.set_number(h % 10, TIME2);
    my_display.set_number(h / 10, TIME1);
    // day of the month
    my_display.set_number(d % 10, DAY2);
    my_display.set_number(d / 10, DAY1);
    // month
    my_display.set_number(mo % 10, MONTH2);
    my_display.set_number(mo / 10, MONTH1);
    // year
    my_display.set_number(y % 10, YEAR2);
    my_display.set_number((y / 10) % 10, YEAR1);

    // the smiley/frowny indicates if the time is valid
    // i.e. the latest NTP update was successful
    if (rtc_data.valid_time == 0)
        my_display.set_shape(FACE_ANGRY);
    else
        my_display.set_shape(FACE_HAPPY);
    // have dots between hours and minutes
    my_display.set_shape(TIMEDOTS);
    // write the screen to the display
    my_display.write_display();
}

void setup()
{
    // allow serial printing
    Serial.begin(115200);

    wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("ConnectAP_10.0.1.1");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    // init time variables with location
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();

    // initialize the e-ink display and do a redraw
    my_display.init();
}

void loop()
{
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.print(second());
    Serial.println();

    uint32_t seconds_to_add = 0;

    if ((rtc_data.secs_since_NTP >= DO_NTP_UPDATE) || (rtc_data.valid_time == 0))
    {
        // do a full screen refresh
        my_display.init();
        // get the current time via NTP
        do_NTP_update();
        // calculate the change in seconds to make the clock change on a whole minute
        // (the +1 is to make sure to not end up with :59 and end up always almost one minute behind)
        seconds_to_add = 60 - second() + 1;
    }

    update_display();

    // sleep for a minute (or after an NTP update possibly somewhat longer)
    // note the update_display takes about 1315ms
    delay((60 + seconds_to_add) * 1000 - 1320);
    // add one minute to the time since the last NTP update
    rtc_data.secs_since_NTP += 60;
}
