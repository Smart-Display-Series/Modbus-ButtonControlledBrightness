/*
*   ButtonControlledBrightness
*     Control brightness using smartdisplay button widgets
*/
#include <ModbusMaster.h>

/*
*   SmartDisplay Modbus Specification
*/
#define SMARTDISPLAY_SLAVE_ADDRESS 0x7B
#define SMARTDISPLAY_BAUDRATE 115200
#define SMARTDISPLAY_BRIGHTNESS 2111

#define MAX_WIDGET 64
#define WIDGET_EXTENDED 10
#define WIDGET_EXTENDED_BASE 10000

// From widget id to Modbus register index
#define SMARTDISPLAY_WIDGET_TYPE(id) (100 * id + 0 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_POS_X(id) (100 * id + 2 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_POS_Y(id) (100 * id + 3 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_STYLE(id) (100 * id + 4 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0)
#define SMARTDISPLAY_WIDGET_SETVALUE(id) (100 * id + 6 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_GETVALUE(id) (100 * id + 7 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_TEXT(id) (100 * id + 8 + +((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))

#define MAX_BRIGHTNESS 100
#define MIN_BRIGHTNESS 0
#define BRIGHTNESS_STEP 1

/*
*   Widget ID
*/
#define WIDGET_INC 0
#define WIDGET_DEC 1
#define WIDGET_BRIGHTNESS 2

/*
*   Modbus object
*/
ModbusMaster _node;

/*
*   Status
*/
uint8_t _brightness = MAX_BRIGHTNESS;  // Default value

/*
*   Functions
*/
uint16_t readWidgetValue(uint16_t id) {
    uint16_t value = 0;
    uint8_t result = _node.readHoldingRegisters(SMARTDISPLAY_WIDGET_GETVALUE(id), 1);
    if (result == _node.ku8MBSuccess) {
        value = _node.getResponseBuffer(0);
    }
    return value;
}

bool writeWidgetValue(uint16_t id, uint16_t value) {
    _node.setTransmitBuffer(0, value);
    uint8_t result = _node.writeMultipleRegisters(SMARTDISPLAY_WIDGET_SETVALUE(id), 1);
    return (value == _node.ku8MBSuccess);
}

void setBrightness(uint8_t brightness) {
    _node.writeSingleRegister(SMARTDISPLAY_BRIGHTNESS, brightness);
    writeWidgetValue(WIDGET_BRIGHTNESS, brightness);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(SMARTDISPLAY_BAUDRATE);
    _node.begin(SMARTDISPLAY_SLAVE_ADDRESS, Serial);
}

void loop() {
    uint8_t result;
    static uint16_t btnInc = 0, btnDec = 0;  // Keep previous button states
    uint16_t value;
    uint8_t brightness = _brightness;

    // Read Inc button
    value = readWidgetValue(WIDGET_INC);
    if (btnInc == 0 && value != 0) {  // Only when the button just pressed
        brightness += BRIGHTNESS_STEP;
        if (brightness > MAX_BRIGHTNESS)
            brightness = MAX_BRIGHTNESS;
    }
    btnInc = value;

    value = readWidgetValue(WIDGET_DEC);
    if (btnDec == 0 && value != 0) {  // Only when the button just pressed
        if (brightness <= BRIGHTNESS_STEP)
            brightness = 0;
        else
            brightness -= BRIGHTNESS_STEP;
    }
    btnDec = value;

    if (_brightness != brightness) {  // When brightness updated
        setBrightness((uint8_t)brightness);
        writeWidgetValue(WIDGET_BRIGHTNESS, brightness);
        _brightness = brightness;
    }
    digitalWrite(LED_BUILTIN, (btnInc != 0) || (btnDec != 0));  // LED is ON if any of the two buttons is pressed
    delay(10);
}
