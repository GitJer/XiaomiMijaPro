/*
 * NOTES:
 * 
 * There needs to be a short between D0 and the RST on the esp8266. This 
 * allows the esp8266 to wake up after the specified Deep-Sleep time. 
 * With this short in place programming is not possible.
 * 
 * During Deep-Sleep all GPIO values go to low, also the reset line to
 * the display. This means that the display thinks it is being reset and
 * reacts wrongly to a write_display. To remedy this, a short between the
 * reset line (see IO_RST_N in XiaomiMijaHetmpHumPro.h) needs to be made. The 
 * consequence is that a regular (say once every hour) reset to clear the 
 * screen of any stray e-ink pixels will not work. I have tried to use a 
 * pull-up resistor and tried values from 4k7 to 220 ohm, but this did not 
 * help.
 * 
 * If the screen remains blanc after startup it may be because the rtc memory 
 * of the WifiManager is not set or somehow corrupted. Using e.g. a mobile 
 * phone to connect to the access point XIAOMI_10.0.1.1 and then in a browser 
 * on the phone go to the website 10.0.1.1 allows you to set the ssid and 
 * passwd.
 * 
 * If the screen has just started up, it may take up to two minutes before the 
 * update occurs every minute.
 * 
 * If the esp is reset during Deep-Sleep, it is still registered as a wakeup 
 * from Deep-Sleep. Power up is not wake up from Deep-Sleep
 */

/*
 * This code is for controlling the e-ink screen of the 
 * xiaomi mijia Mi miaomiaoce Thermometer Temperature Humidity Sensor 
 * 
 * See  https://github.com/PaulStoffregen/Time for the NTP stuff
 * See  https://github.com/tzapu/WiFiManager for the wifi credentials manager
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "XiaomiMijaTempHumPro.h"

// uncomment to see debug information via Serial
// #define DEBUG

// uncomment to use WiFiManager, if commented provide ssid and password
// #define USE_WIFIMANAGER

#ifndef USE_WIFIMANAGER
// The SSID (name) of the Wi-Fi network you want to connect to
const char *ssid = "Your SSID";
// The password of the Wi-Fi network
const char *password = "Your password";
#endif

// update via NTP once every DO_NTP_UPDATE seconds (typically an hour)
#define DO_NTP_UPDATE 3600
// change the clock once every SECONDS_DEEP_SLEEP seconds
#define SECONDS_DEEP_SLEEP 60

// display
XiaomiMijaTempHumPro my_display;

// wifimanager
WiFiManager wifiManager;
// NTP Server
static const char ntpServerName[] = "nl.pool.ntp.org";
const int timeZone = 1; // Central European Time
// UDP settings
WiFiUDP Udp;
// local port to listen for UDP packets
unsigned int localPort = 8888;

// struct to save variables
struct
{
    time_t unix_time = 0;
    // the number of seconds since the last NTP update
    int secs_since_NTP = DO_NTP_UPDATE;
    // indicate if the NTP update was unsuccessful
    int valid_time = 0;
} rtc_data;

// NTP stuff

// NTP time is in the first 48 bytes of message
const int NTP_PACKET_SIZE = 48;
//buffer to hold incoming & outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
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
    // NTP requests are to port 123
    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

uint32_t getNtpTime()
{
    // NTP server's ip address
    IPAddress ntpServerIP;

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
            // read packet into the buffer
            Udp.read(packetBuffer, NTP_PACKET_SIZE);
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            // 2208988800UL = the 70 years from 1900 to 1970
            rtc_data.unix_time = secsSince1900 - 2208988800UL;
            // valid response
            return 1;
        }
    }
    // return 0 if unable to get the time
    return 0;
}

// Get the time via NTP
void do_NTP_update()
{

#ifdef USE_WIFIMANAGER
    // make connection with wifi
    wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
    wifiManager.autoConnect("XIAOMI_10.0.1.1");
#else
    WiFi.begin(ssid, password);
    int wifi_try_count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        // add one second to the unix_time
        rtc_data.unix_time += 1;
        // max number of tries
        wifi_try_count++;
#ifdef DEBUG
        Serial.print("wifi connect try:");
        Serial.println(wifi_try_count);
#endif
        if (wifi_try_count > 10)
            break;
    }
#endif

#ifdef DEBUG
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("No WiFi connection");
    }
#endif

    if (WiFi.status() == WL_CONNECTED)
    {
        // start UDP
        Udp.begin(localPort);
        // get the time in seconds since 1970
        uint32_t my_time = getNtpTime();
        if (my_time == 1)
        {
            // there was a valid response from the NTP server
            rtc_data.valid_time = 1;
            rtc_data.secs_since_NTP = 0;
            WiFi.disconnect();
            return;
        }
    }
    // no wifi or the NTP server did not respond
    rtc_data.valid_time = 0;
    // do a disconnect from wifi
    WiFi.disconnect();
}

// =========================================================== DISPLAY SETTINGS

void update_display()
{
    // display the time

    // start building a new screen
    my_display.start_new_screen();

    // get the current time
    tm *normal_time;
    normal_time = localtime(&rtc_data.unix_time);
    int m = normal_time->tm_min;
    int h = normal_time->tm_hour;
    int d = normal_time->tm_mday;
    int mo = normal_time->tm_mon + 1;
    int y = normal_time->tm_year;

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
#ifdef DEBUG
    Serial.println("doing write_display");
#endif
    my_display.write_display();
}

void setup()
{
#ifdef DEBUG
    // allow serial printing
    Serial.begin(115200);
#endif
    // init time variables with location
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();

    String reset_reason = ESP.getResetReason();

    if (reset_reason != "Deep-Sleep Wake")
    {
        // not a Deep-Sleep wake: initialize

        // initialize the e-ink display and do a redraw
        my_display.init(1);

        // init time variables
        rtc_data.unix_time = 0;
        rtc_data.secs_since_NTP = DO_NTP_UPDATE + 1;
        rtc_data.valid_time = 0;

        // get the current time via NTP
        do_NTP_update();
    }
    else
    {
        // it is a wake-up from Deep-Sleep

        // read variables from rtc memory
        ESP.rtcUserMemoryRead(0, (uint32_t *)&rtc_data, sizeof(rtc_data));
        // start the display without redraw of the screen
        my_display.init(0);

        // is it time for a NTP update? If not: continue with current time values
        rtc_data.unix_time += SECONDS_DEEP_SLEEP;
        rtc_data.secs_since_NTP += SECONDS_DEEP_SLEEP;
        if (rtc_data.secs_since_NTP > DO_NTP_UPDATE)
        {
            // get the current time via NTP
            do_NTP_update();
            // also do a display redraw (cleans up stray black/white e-ink elements)
            // Odly it works eventhough the RST_N line is always HIGH (connected to 3V3)
            my_display.init(1);
        }
    }

    // display the current time and date on the display
    update_display();

    // calculate extra seconds to make the Deep-Sleep wake up on a whole minute
    tm *normal_time;
    normal_time = localtime(&rtc_data.unix_time);
    uint32_t seconds_to_add = (60 - normal_time->tm_sec) % 60;
    // add the extra seconds to unix_time.
    // Note: during wake-up SECONDS_DEEP_SLEEP are added to unix_time
    rtc_data.unix_time += seconds_to_add;
    // sleep for 60 seconds + the extra seconds
    unsigned long seconds_deep_sleep = SECONDS_DEEP_SLEEP + seconds_to_add;

    // write the variables that need to survive Deep-Sleep
    ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtc_data, sizeof(rtc_data));
    // go to sleep
    // Note: radio should be started at wake-up else it wont start
    ESP.deepSleep(seconds_deep_sleep * 1e6, WAKE_RF_DEFAULT);
}

// not used
void loop()
{
}
