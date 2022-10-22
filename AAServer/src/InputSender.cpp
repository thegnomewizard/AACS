#include "InputSender.h"

namespace {

const uint8_t touchScreenHidDescriptor[] = {
        // 0x05, 0x0d,                    // USAGE_PAGE (Digitizer)
        // 0x09, 0x02,                    // USAGE (Pen)
        // 0xa1, 0x01,                    // COLLECTION (Application)
		
		// // declare a finger collection
        // 0x09, 0x20,                    //   Usage (Stylus)
        // 0xA1, 0x00,                    //   Collection (Physical)

        // // Declare a finger touch (finger up/down)
        // 0x09, 0x42,                    //     Usage (Tip Switch)
		// 0x09, 0x32,                    //     USAGE (In Range)
        // 0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
        // 0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
        // 0x75, 0x01,                    //     REPORT_SIZE (1)
        // 0x95, 0x02,                    //     REPORT_COUNT (2)
        // 0x81, 0x02,                    //     INPUT (Data,Var,Abs)

        // // Declare the remaining 6 bits of the first data byte as constant -> the driver will ignore them
        // 0x75, 0x01,                    //     REPORT_SIZE (1)
        // 0x95, 0x06,                    //     REPORT_COUNT (6)
        // 0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)

        // // Define absolute X and Y coordinates of 16 bit each (percent values multiplied with 100)
        // // http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
        // // Chapter 16.2 says: "In the Stylus collection a Pointer physical collection will contain the axes reported by the stylus."
        // 0x05, 0x01,                    //     Usage Page (Generic Desktop)
        // 0x09, 0x01,                    //     Usage (Pointer)
        // 0xA1, 0x00,                    //     Collection (Physical)
        // 0x09, 0x30,                    //        Usage (X)
        // 0x09, 0x31,                    //        Usage (Y)
        // 0x16, 0x00, 0x00,              //        Logical Minimum (0)
        // 0x26, 0x10, 0x27,              //        Logical Maximum (10000)
        // 0x36, 0x00, 0x00,              //        Physical Minimum (0)
        // 0x46, 0x10, 0x27,              //        Physical Maximum (10000)
        // 0x66, 0x00, 0x00,              //        UNIT (None)
        // 0x75, 0x10,                    //        Report Size (16),
        // 0x95, 0x02,                    //        Report Count (2),
        // 0x81, 0x02,                    //        Input (Data,Var,Abs)
        // 0xc0,                          //     END_COLLECTION

        // 0xc0,                          //   END_COLLECTION
        // 0xc0                           // END_COLLECTION

        // // With this declaration a data packet must be sent as:
        // // byte 1   -> "touch" state          (bit 0 = pen up/down, bit 1 = In Range)
        // // byte 2,3 -> absolute X coordinate  (0...10000)
        // // byte 4,5 -> absolute Y coordinate  (0...10000)
        /* try 1b */
        0x05, 0x0d,                    // USAGE_PAGE (Digitizer)
        0x09, 0x04,                    // USAGE (Touch Screen)
        0xa1, 0x01,                    // COLLECTION (Application)
		
		// declare a finger collection
        0x09, 0x20,                    //   Usage (Stylus)
//        0xA1, 0x00,                    //   Collection (Physical)
0xa1,0x01,
        // Declare a finger touch (finger up/down)
        0x09, 0x42,                    //     Usage (Tip Switch)
		0x09, 0x32,                    //     USAGE (In Range)
        0x09, 0x44, // USAGE (Barrel Switch) [FOR LIBINPUT STUPIDITY]
        0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
        0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
        0x75, 0x01,                    //     REPORT_SIZE (1)
        0x95, 0x03,                    //     REPORT_COUNT (3)
        0x81, 0x02,                    //     INPUT (Data,Var,Abs)

        // Declare the remaining 6 bits of the first data byte as constant -> the driver will ignore them
        0x75, 0x01,                    //     REPORT_SIZE (1)
        0x95, 0x05,                    //     REPORT_COUNT (5)
        0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)

        // Define absolute X and Y coordinates of 16 bit each (percent values multiplied with 100)
        // http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
        // Chapter 16.2 says: "In the Stylus collection a Pointer physical collection will contain the axes reported by the stylus."
        0x05, 0x01,                    //     Usage Page (Generic Desktop)
        0x09, 0x01,                    //     Usage (Pointer)
        0xA1, 0x00,                    //     Collection (Physical)
        0x09, 0x30,                    //        Usage (X)
        0x09, 0x31,                    //        Usage (Y)
        0x16, 0x00, 0x00,              //        Logical Minimum (0)
        0x26, 0x10, 0x27,              //        Logical Maximum (10000)
        0x36, 0x00, 0x00,              //        Physical Minimum (0)
        0x46, 0x10, 0x27,              //        Physical Maximum (10000)
//        0x66, 0x00, 0x00,              //        UNIT (None)
0x65,0x11,
        0x75, 0x10,                    //        Report Size (16),
        0x95, 0x02,                    //        Report Count (2),
        0x81, 0x02,                    //        Input (Data,Var,Abs)
        0xc0,                          //     END_COLLECTION

        0xc0,                          //   END_COLLECTION
        0xc0                           // END_COLLECTION

        // With this declaration a data packet must be sent as:
        // byte 1   -> "touch" state          (bit 0 = pen up/down, bit 1 = In Range)
        // byte 2,3 -> absolute X coordinate  (0...10000)
        // byte 4,5 -> absolute Y coordinate  (0...10000)
        /* try 2 */
    // 0x05, 0x0d,                         // USAGE_PAGE (Digitizers)
    // 0x09, 0x04,                         // USAGE (Touch Screen)
    // 0xa1, 0x01,                         // COLLECTION (Application)
    // 0x85, 4,                            //   REPORT_ID (Touch)
    // 0x09, 0x20,                         //   USAGE (Stylus)
    // 0xa1, 0x00,                         //   COLLECTION (Physical)
    // 0x09, 0x42,                         //     USAGE (Tip Switch)
    // 0x15, 0x00,                         //     LOGICAL_MINIMUM (0)
    // 0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    // 0x75, 0x01,                         //     REPORT_SIZE (1)
    // 0x95, 0x01,                         //     REPORT_COUNT (1)
    // 0x81, 0x02,                         //     INPUT (Data,Var,Abs)
    // 0x95, 0x03,                         //     REPORT_COUNT (3)
    // 0x81, 0x03,                         //     INPUT (Cnst,Ary,Abs)
    // 0x09, 0x32,                         //     USAGE (In Range)
    // 0x09, 0x47,                         //     USAGE (Confidence)
    // 0x95, 0x02,                         //     REPORT_COUNT (2)
    // 0x81, 0x02,                         //     INPUT (Data,Var,Abs)
    // 0x95, 0x0a,                         //     REPORT_COUNT (10)
    // 0x81, 0x03,                         //     INPUT (Cnst,Ary,Abs)
    // 0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop)
    // 0x26, 0xff, 0x7f,                   //     LOGICAL_MAXIMUM (32767)
    // 0x75, 0x10,                         //     REPORT_SIZE (16)
    // 0x95, 0x01,                         //     REPORT_COUNT (1)
    // 0xa4,                               //     PUSH
    // 0x55, 0x0d,                         //     UNIT_EXPONENT (-3)
    // 0x65, 0x00,                         //     UNIT (None)
    // 0x09, 0x30,                         //     USAGE (X)
    // 0x35, 0x00,                         //     PHYSICAL_MINIMUM (0)
    // 0x46, 0x00, 0x00,                   //     PHYSICAL_MAXIMUM (0)
    // 0x81, 0x02,                         //     INPUT (Data,Var,Abs)
    // 0x09, 0x31,                         //     USAGE (Y)
    // 0x46, 0x00, 0x00,                   //     PHYSICAL_MAXIMUM (0)
    // 0x81, 0x02,                         //     INPUT (Data,Var,Abs)
    // 0xb4,                               //     POP
    // 0x05, 0x0d,                         //     USAGE PAGE (Digitizers)
    // 0x09, 0x48,                         //     USAGE (Width)
    // 0x09, 0x49,                         //     USAGE (Height)
    // 0x95, 0x02,                         //     REPORT_COUNT (2)
    // 0x81, 0x02,                         //     INPUT (Data,Var,Abs)
    // 0x95, 0x01,                         //     REPORT_COUNT (1)
    // 0x81, 0x03,                         //     INPUT (Cnst,Ary,Abs)
    // 0xc0,                               //   END_COLLECTION
    // 0xc0,                               // END_COLLECTION
};

void Set(uint8_t* pos, uint16_t value)
{
    pos[1] = (value >> 8) & 0xFF;
    pos[0] = (value >> 0) & 0xFF;
}

}

InputSender::InputSender()
{
    _driver.Create("AutoScreen", 0x1234, 0x5678, touchScreenHidDescriptor, sizeof(touchScreenHidDescriptor));
}

void InputSender::SetScreenSize(int width, int height)
{
    _width = width;
    _height = height;
}

void InputSender::SendEvent(bool active, int x, int y)
{
    uint8_t packet[5];
    packet[0] = active ? 3 : 0;
    Set(&packet[1], ((uint64_t(x) * 10000) / _width));
    Set(&packet[3], ((uint64_t(y) * 10000) / _height));
    _driver.Send(packet, sizeof(packet));
}
