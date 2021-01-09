#include "XiaomiMijaTempHumPro.h"

/*
    Version 0.0.1
*/

//----------------------------------
// Various LUT and initial data values taken from the actual device with a logic analyzer
//----------------------------------

uint8_t S_dummy[1] = {0x00};
uint8_t S_20_init[15] = {0x4A, 0x4A, 0x01, 0x8A, 0x8A, 0x01, 0x4A, 0x4A, 0x01, 0x8A, 0x8A, 0x01, 0x81, 0x81, 0x01};
uint8_t S_20_normal[15] = {0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x81, 0x81, 0x01};
uint8_t S_23_init[15] = {0x8A, 0x8A, 0x01, 0x8A, 0x8A, 0x01, 0x4A, 0x4A, 0x01, 0x4A, 0x4A, 0x01, 0x81, 0x81, 0x01};
uint8_t S_23_normal[15] = {0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x81, 0x81, 0x01};
uint8_t S_24_normal[15] = {0x46, 0x46, 0x01, 0x46, 0x46, 0x01, 0x86, 0x86, 0x01, 0x86, 0x86, 0x01, 0x81, 0x81, 0x01};
uint8_t S_25_normal[15] = {0x86, 0x86, 0x01, 0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x46, 0x46, 0x01, 0x81, 0x81, 0x01};
uint8_t S_26_init_1[15] = {0x4A, 0x4A, 0x01, 0x4A, 0x4A, 0x01, 0x8A, 0x8A, 0x01, 0x8A, 0x8A, 0x01, 0x81, 0x81, 0x01};
uint8_t S_26_init_2[15] = {0x46, 0x46, 0x01, 0x46, 0x46, 0x01, 0x86, 0x86, 0x01, 0x86, 0x86, 0x01, 0x81, 0x81, 0x01};
uint8_t S_26_normal[15] = {0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x86, 0x86, 0x01, 0x46, 0x46, 0x01, 0x81, 0x81, 0x01};
uint8_t S_18_init_1[18] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t S_18_init_2[18] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t S_1C_init_1[18] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t S_1C_init_2[18] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//----------------------------------
// define segments for the groups
// the data in the arrays consists of {byte, bit} pairs of each segment
//----------------------------------
// big number top-left
uint8_t time1[14] = {14, 7, 12, 4, 11, 7, 13, 5, 15, 2, 15, 1, 14, 5};
// big number right to time1
uint8_t time2[14] = {13, 2, 12, 6, 9, 2, 10, 4, 10, 3, 12, 5, 10, 5};
// big number right to time2
uint8_t time3[14] = {13, 1, 11, 0, 9, 5, 5, 4, 9, 1, 12, 7, 10, 7};
// big number right to time3
uint8_t time4[14] = {13, 0, 11, 2, 7, 0, 5, 3, 9, 6, 11, 1, 10, 6};
// two dots between time2 and time3
uint8_t timedots[4] = {11, 4, 9, 0};
// the happy face below time4
uint8_t face_happy[8] = {2, 2, 3, 0, 3, 1, 2, 3};
// the angry face below time4
uint8_t face_angry[8] = {2, 2, 3, 0, 2, 3, 3, 7};
// two digits below time1
uint8_t day1[14] = {13, 3, 13, 7, 13, 6, 0, 0, 14, 2, 14, 4, 14, 3};
uint8_t day2[14] = {11, 6, 11, 5, 12, 2, 1, 7, 12, 0, 12, 1, 12, 3};
// two digits below time2
uint8_t month1[14] = {8, 1, 8, 5, 8, 6, 8, 3, 9, 4, 9, 3, 9, 7};
uint8_t month2[14] = {6, 3, 6, 6, 6, 7, 6, 0, 7, 5, 7, 4, 6, 5};
// two digits below time3
uint8_t year1[14] = {4, 2, 4, 3, 1, 4, 1, 5, 1, 6, 4, 1, 3, 6};
uint8_t year2[14] = {4, 5, 4, 6, 1, 1, 1, 2, 1, 3, 4, 4, 2, 1};

// Define how the numbers map to the segments:
//
//      1
//  6       2
//      7
//  5       3
//      4
//
int numbers[10][7] = {
    {1, 2, 3, 4, 5, 6, 0}, // 0
    {2, 3, 0, 0, 0, 0, 0}, // 1
    {1, 2, 7, 5, 4, 0, 0}, // 2
    {1, 2, 3, 4, 7, 0, 0}, // 3
    {6, 7, 2, 3, 0, 0, 0}, // 4
    {1, 6, 7, 3, 4, 0, 0}, // 5
    {1, 3, 4, 5, 6, 7, 0}, // 6
    {1, 2, 3, 0, 0, 0, 0}, // 7
    {1, 2, 3, 4, 5, 6, 7}, // 8
    {1, 2, 3, 4, 6, 7, 0}  // 9
};

void XiaomiMijaTempHumPro::init()
{
    // set the pin modes (note: no hardware SPI is used)
    pinMode(SPI_ENABLE, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    pinMode(IO_RST_N, OUTPUT);
    pinMode(SPI_CLOCK, OUTPUT);
    pinMode(IO_BUSY_N, INPUT);

    // set all output to default
    digitalWrite(SPI_ENABLE, 0);
    digitalWrite(SPI_MOSI, 0);
    digitalWrite(IO_RST_N, 1);
    digitalWrite(SPI_CLOCK, 0);

    // disable SPI (SPI enable is low active)
    digitalWrite(SPI_ENABLE, 1);

    delay(100);
    // give a RST pulse
    // digitalWrite(IO_RST_N, 0);
    // delayMicroseconds(100);
    // digitalWrite(IO_RST_N, 1);

    // // do the first block of init data.
    // transmit(0, CMD_04);
    // delay(100);
    // while (digitalRead(IO_BUSY_N) == 0)
    //     delay(1);
    // transmit(0, CMD_00);
    // transmit(1, 0x0B);
    // transmit(0, CMD_01);
    // transmit(1, 0x46);
    // transmit(1, 0x46);
    // transmit(0, CMD_03);
    // transmit(1, 0x00);
    // transmit(0, CMD_30);
    // transmit(1, 0x02);
    // transmit(0, CMD_20);
    // for (int i = 0; i < 15; i++)
    //     transmit(1, S_20_init[i]);
    // transmit(0, CMD_23);
    // for (int i = 0; i < 15; i++)
    //     transmit(1, S_23_init[i]);
    // transmit(0, CMD_26);
    // for (int i = 0; i < 15; i++)
    //     transmit(1, S_26_init_1[i]);
    // transmit(0, CMD_18);
    // for (int i = 0; i < 18; i++)
    //     transmit(1, S_18_init_1[i]);
    // transmit(0, CMD_1C);
    // for (int i = 0; i < 18; i++)
    //     transmit(1, S_1C_init_1[i]);
    // transmit(0, CMD_12);
    // delay(200);
    // while (digitalRead(IO_BUSY_N) == 0)
    //     delay(1);
    // transmit(0, CMD_02);
    // transmit(1, 0x03);
    // delay(100);

    // // give a second RST pulse
    digitalWrite(IO_RST_N, 0);
    delayMicroseconds(100);
    digitalWrite(IO_RST_N, 1);

    // do the first block of init data.
    transmit(0, CMD_04);
    delay(100);
    while (digitalRead(IO_BUSY_N) == 0)
        delay(1);
    transmit(0, CMD_00);
    transmit(1, 0x0B);
    transmit(0, CMD_01);
    transmit(1, 0x46);
    transmit(1, 0x46);
    transmit(0, CMD_03);
    transmit(1, 0x00);
    transmit(0, CMD_30);
    transmit(1, 0x02);
    transmit(0, CMD_20);
    for (int i = 0; i < 15; i++)
        transmit(1, S_20_normal[i]);
    transmit(0, CMD_23);
    for (int i = 0; i < 15; i++)
        transmit(1, S_23_normal[i]);
    transmit(0, CMD_26);
    for (int i = 0; i < 15; i++)
        transmit(1, S_26_init_2[i]);
    transmit(0, CMD_18);
    for (int i = 0; i < 18; i++)
        transmit(1, S_18_init_2[i]);
    transmit(0, CMD_1C);
    for (int i = 0; i < 18; i++)
        transmit(1, S_1C_init_2[i]);
    transmit(0, CMD_12);
    delay(200);
    while (digitalRead(IO_BUSY_N) == 0)
        delay(1);
    transmit(0, CMD_02);
    transmit(1, 0x03);
    delay(100);
}

void XiaomiMijaTempHumPro::write_display()
{
    transmit(0, CMD_04);
    delay(100);
    while (digitalRead(IO_BUSY_N) == 0)
        delay(1);

    transmit(0, CMD_00);
    transmit(1, 0x0B);
    transmit(0, CMD_01);
    transmit(1, 0x46);
    transmit(1, 0x46);
    transmit(0, CMD_03);
    transmit(1, 0x06);
    transmit(0, CMD_30);
    transmit(1, 0x03);
    transmit(0, CMD_15);
    transmit(1, 0x00);
    transmit(1, 0x87);
    transmit(1, 0x01);

    transmit(0, CMD_20);
    for (int i = 0; i < 15; i++)
        transmit(1, S_20_normal[i]);
    transmit(0, CMD_23);
    for (int i = 0; i < 15; i++)
        transmit(1, S_23_normal[i]);
    transmit(0, CMD_24);
    for (int i = 0; i < 15; i++)
        transmit(1, S_24_normal[i]);
    transmit(0, CMD_25);
    for (int i = 0; i < 15; i++)
        transmit(1, S_25_normal[i]);
    transmit(0, CMD_26);
    for (int i = 0; i < 15; i++)
        transmit(1, S_26_normal[i]);
    transmit(0, CMD_18);
    for (int i = 0; i < 18; i++)
        transmit(1, display_data[i]);

    transmit(0, CMD_12);
    delay(200);
    while (digitalRead(IO_BUSY_N) == 0)
        delay(1);

    transmit(0, CMD_02);
    transmit(1, 0x03);
    delay(50);
}

void XiaomiMijaTempHumPro::transmit(uint8_t cd, uint8_t data_to_send)
{
    // enable SPI
    digitalWrite(SPI_ENABLE, 0);
    delayMicroseconds(10);

    // send the first bit, this indicates if the following is a command or data
    digitalWrite(SPI_CLOCK, 0);
    if (cd != 0)
        digitalWrite(SPI_MOSI, 1);
    else
        digitalWrite(SPI_MOSI, 0);
    delayMicroseconds(10);
    digitalWrite(SPI_CLOCK, 1);
    delayMicroseconds(10);

    // send 8 bits
    for (int i = 0; i < 8; i++)
    {
        // start the clock cycle
        digitalWrite(SPI_CLOCK, 0);
        // set the MOSI according to the data
        if (data_to_send & 0x80)
            digitalWrite(SPI_MOSI, 1);
        else
            digitalWrite(SPI_MOSI, 0);
        // prepare for the next bit
        data_to_send = (data_to_send << 1);
        delayMicroseconds(10);
        // the data is read at rising clock (halfway the time MOSI is set)
        digitalWrite(SPI_CLOCK, 1);
        delayMicroseconds(10);
    }

    // finish by ending the clock cycle and disabling SPI
    digitalWrite(SPI_CLOCK, 0);
    delayMicroseconds(10);
    digitalWrite(SPI_ENABLE, 1);
    delayMicroseconds(10);
}

void XiaomiMijaTempHumPro::set_number(uint8_t number, uint8_t where)
{
    // 'number' must be 0 to 9
    number = number % 10;
    // 'where' must one of: TIME_1, TIME_2, TIME_3, TIME_4, DAY1, DAY2, MONTH1, MONTH2, YEAR1, or YEAR2.
    uint8_t *segments;
    switch (where)
    {
    case TIME1:
        segments = time1;
        break;
    case TIME2:
        segments = time2;
        break;
    case TIME3:
        segments = time3;
        break;
    case TIME4:
        segments = time4;
        break;
    case DAY1:
        segments = day1;
        break;
    case DAY2:
        segments = day2;
        break;
    case MONTH1:
        segments = month1;
        break;
    case MONTH2:
        segments = month2;
        break;
    case YEAR1:
        segments = year1;
        break;
    case YEAR2:
        segments = year2;
        break;
    default:
        break;
    }

    // set the segments, there are up to 7 segments in a number
    int segment_byte;
    int segment_bit;
    for (int i = 0; i < 7; i++)
    {
        // get the segment needed to display the number 'number',
        // this is stored in the array 'numbers'
        int segment = numbers[number][i] - 1;
        // segment = -1 indicates there are no more segments to display
        if (segment == -1)
            break;
        segment_byte = segments[2 * segment];
        segment_bit = segments[1 + 2 * segment];
        set_segment(segment_byte, segment_bit);
    }
}

void XiaomiMijaTempHumPro::set_shape(uint8_t where)
{
    int num_of_segments = 0;
    uint8_t *segments = NULL;

    // set the number of segments and which segments has to be displayed
    switch (where)
    {
    case TIMEDOTS:
        num_of_segments = 2;
        segments = timedots;
        break;
    case FACE_HAPPY:
        num_of_segments = 5;
        segments = face_happy;
        break;
    case FACE_ANGRY:
        num_of_segments = 4;
        segments = face_angry;
        break;
    default:
        return;
    }

    // set the segments
    for (uint8_t segment = 0; segment < num_of_segments; segment++)
    {
        uint8_t segment_byte = segments[2 * segment];
        uint8_t segment_bit = segments[1 + 2 * segment];
        set_segment(segment_byte, segment_bit);
    }
}

void XiaomiMijaTempHumPro::set_segment(uint8_t segment_byte, uint8_t segment_bit)
{
    // set the bit
    display_data[segment_byte] |= (1 << segment_bit);
}

void XiaomiMijaTempHumPro::start_new_screen()
{
    // prepare the data to be displayed, assume all segments are off
    for (int i = 0; i < 17; i++)
        display_data[i] = 0x00;
    // don't know why, but the last byte is 0xFF
    display_data[17] = 0xFF;
}