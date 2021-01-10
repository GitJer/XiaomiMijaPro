#ifndef __XiaomiMijaTempHumPro_H
#define __XiaomiMijaTempHumPro_H

/*
    Version 0.0.1
*/

#include <stdint.h>
#include <Arduino.h>

//----------------------------------
// define pins (made such that it can easily connect the ESP8266 Wemos d1 mini
// to the pin-to-flatflex connector)
//----------------------------------
#define SPI_ENABLE D7
#define SPI_MOSI D6
#define IO_RST_N D5
#define SPI_CLOCK D1
#define IO_BUSY_N D2

//----------------------------------
// define commands and settings
//----------------------------------
#define CMD_00 0x00
#define CMD_01 0x01
#define CMD_02 0x02
#define CMD_03 0x03
#define CMD_04 0x04
#define CMD_12 0x12
#define CMD_15 0x15
#define CMD_18 0x18
#define CMD_1C 0x1C
#define CMD_20 0x20
#define CMD_23 0x23
#define CMD_24 0x24
#define CMD_25 0x25
#define CMD_26 0x26
#define CMD_30 0x30

//----------------------------------
// define groups of segments into logical shapes
// Note: first all the number types, then the shapes!
//----------------------------------
#define TIME1 1  // the high hours digit, example: time=13:48 -> 1
#define TIME2 2  // the low hours digit, example: time=13:48 -> 3
#define TIME3 3  // the high minutes digit, example: time=13:48 -> 4
#define TIME4 4  // the low minutes digit, example: time=13:48 -> 8
#define DAY1 5   // the high day digit, example: date= 30th of december -> 3
#define DAY2 6   // the low day digit, example: date= 30th of december -> 0
#define MONTH1 7 // the high month digit, example: date= 30th of december -> 1
#define MONTH2 8 // the low month digit, example: date= 30th of december -> 2
#define YEAR1 9  // the high year digit (only last two digits of the year are used) example: 2021 -> 2
#define YEAR2 10 // the low year digit, example: 2021 -> 1

#define TIMEDOTS 11
#define FACE_HAPPY 12
#define FACE_ANGRY 13

//----------------------------------
// define the class for driving the Xiaomi MiaoMiaoCe e-ink display
//----------------------------------
class XiaomiMijaTempHumPro
{
public:
    /*
     * Initialize the display
     * @param redraw    should the screen should have an black-white redraw?
     *                  redraw = 0: no redraw
     *                  redraw != 0: do a redraw 
     *                  normally redrawing is advisable (redraw != 0), 
     *                  but e.g. when using deep-sleep you may want to
     *                  initialize the screen without the black-white 
     *                  transition
     */
    void init(int redraw);

    /* 
     * Start building a new display
    */
    void start_new_screen();

    /*
     * send the data on the display
     */
    void write_display();

    /* 
     * Display a number at a specific place
     * @param number    The number to be displayed [0, 9]
     * @param where     The location where to display the number: must be 
     *                  one of:
     *                  TIME_1, TIME_2, TIME_3, TIME_4, DAY1, DAY2, MONTH1,
     *                  MONTH2, YEAR1, or YEAR2 
     */
    void set_number(uint8_t number, uint8_t where);

    /* 
     * Display a defined shape
     * @param where     Which shapes should be turned on, there are several 
     *                  pre-defined shapes: see above in this file.
     *                  But if you want you can define your own, see the 
     *                  #defines above and the variables in XiaomiMijaTempHumPro.cpp
     */
    void set_shape(uint8_t where);

    /*
     * Set a segment to a value
     * @param segment_byte  The byte in which the segments is located
     * @param segment_bit   The bit in the byte
     */
    void set_segment(uint8_t segment_byte, uint8_t segment_bit);

private:
    /*
     * Transmit data to the display via SPI
     * @param cd    is it a command or data? 0 = command, 1 = data
     * @param data  1 byte of data
     */
    void transmit(uint8_t cd, uint8_t data);

    // The array in which the segments to be displayed are placed
    uint8_t display_data[18];
};
#endif