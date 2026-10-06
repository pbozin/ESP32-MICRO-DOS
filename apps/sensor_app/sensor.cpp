#include "microdos_api.h"
#include "microdos_math.h"

// I2C Pins
#define SDA_PIN 32 
#define SCL_PIN 25

// Bosch BME680 / BME688 Registers & Configuration
#define SENSOR_ADDR       0x77
#define REG_CHIP_ID       0xD0
#define REG_CTRL_HUM      0x72
#define REG_CTRL_MEAS     0x74
#define REG_CTRL_GAS_1    0x71
#define REG_DATA_START    0x1D

// Global State
static uint16_t ALIGNED dig_T1;
static int16_t  ALIGNED dig_T2;
static int16_t  ALIGNED dig_T3;
static uint16_t ALIGNED dig_P1;
static int16_t  ALIGNED dig_P2;
static int16_t  ALIGNED dig_P3;
static int16_t  ALIGNED dig_P4;
static int16_t  ALIGNED dig_P5;
static int16_t  ALIGNED dig_P6;
static int16_t  ALIGNED dig_P7;
static int16_t  ALIGNED dig_P8;
static int16_t  ALIGNED dig_P9;
static int      accent_color ALIGNED = CYAN;
static int32_t  ALIGNED t_fine = 0;

static MicroDosAPI* os ALIGNED = NULL;

// ============================================================================
//   BIT-BANGED I2C PROTOCOL DRIVER
// ============================================================================

WEAK void i2c_delay() {
    volatile int count = 80;
    while(count--) { __asm__("nop"); }
}

WEAK void i2c_sda_high() {
    os->pinMode(SDA_PIN, INPUT_PULLUP);
    i2c_delay();
}

WEAK void i2c_sda_low() {
    os->pinMode(SDA_PIN, OUTPUT);
    os->digitalWrite(SDA_PIN, 0);
    i2c_delay();
}

WEAK void i2c_scl_high() {
    os->pinMode(SCL_PIN, INPUT_PULLUP);
    i2c_delay();

    volatile int timeout = 1000;
    while (os->digitalRead(SCL_PIN) == 0 && timeout > 0) {
        timeout--;
        i2c_delay();
    }
}

WEAK void i2c_scl_low() {
    os->pinMode(SCL_PIN, OUTPUT);
    os->digitalWrite(SCL_PIN, 0);
    i2c_delay();
}

// ============================================================================
//   PROTOCOL SIGNALS
// ============================================================================

WEAK void i2c_init_bus() {
    i2c_sda_high();
    i2c_scl_high();
}

WEAK void i2c_start() {
    i2c_sda_high();
    i2c_scl_high();
    i2c_sda_low();
    i2c_scl_low();
}

WEAK void i2c_stop() {
    i2c_sda_low();
    i2c_scl_high();
    i2c_sda_high();
}

WEAK bool i2c_write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (byte & 0x80) i2c_sda_high();
        else i2c_sda_low();
        byte <<= 1;
        i2c_scl_high();
        i2c_scl_low();
    }

    i2c_sda_high();
    i2c_scl_high();

    bool ack ALIGNED = (os->digitalRead(SDA_PIN) == 0);

    i2c_scl_low();
    return ack;
}

WEAK uint8_t i2c_read_byte(bool send_ack) {
    uint8_t byte ALIGNED = 0;

    i2c_sda_high();

    for (int i = 0; i < 8; i++) {
        i2c_scl_high();
        byte <<= 1;
        if (os->digitalRead(SDA_PIN)) {
            byte |= 0x01;
        }
        i2c_scl_low();
    }

    if (send_ack) i2c_sda_low();
    else i2c_sda_high();

    i2c_scl_high();
    i2c_scl_low();
    i2c_sda_high();

    return byte;
}

ALWAYS INLINE bool sensor_write_reg(uint8_t reg, uint8_t value) {
    i2c_start();
    if (!i2c_write_byte(SENSOR_ADDR << 1)) { i2c_stop(); return false; }
    if (!i2c_write_byte(reg))              { i2c_stop(); return false; }
    if (!i2c_write_byte(value))            { i2c_stop(); return false; }
    i2c_stop();
    return true;
}

ALWAYS INLINE bool sensor_read_bytes(uint8_t reg, uint8_t* buffer, size_t length) {
    i2c_start();
    if (!i2c_write_byte(SENSOR_ADDR << 1)) { i2c_stop(); return false; }
    if (!i2c_write_byte(reg))              { i2c_stop(); return false; }
    
    i2c_start();
    if (!i2c_write_byte((SENSOR_ADDR << 1) | 1)) { i2c_stop(); return false; }
    
    for (size_t i = 0; i < length; i++) {
        buffer[i] = i2c_read_byte(i < (length - 1));
    }
    i2c_stop();
    return true;
}

// ============================================================================
//   BOSCH MATH COMPENSATION CONVERTERS
// ============================================================================

WEAK float compensate_temp(int32_t adc_T) {
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = ((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3) >> 14;
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0f;
}

WEAK float compensate_press(int32_t adc_P) {
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) return 0.0f;
    
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (float)p / 256.0f / 100.0f;
}

// ============================================================================
//   UI ENGINE & VISUAL DRAWING
// ============================================================================

WEAK void draw_dashboard(float temp, float press, bool sensor_found) {
    static char str_buf[16] ALIGNED;
    printAt(6, 3, STRING("-- BME68X METRICS LAB --"));

    if (sensor_found) {
        printAt(6, 8, STRING("STATUS: SENSOR NOT FOUND"));
        printAt(6, 10, STRING("Please check SDA and SCL"));
        return;
    }

    os->printAt(1,  8, STRING("TEMPERATURE (C):"));
    itoa(123, str_buf);
    printAt(18, 8, str_buf);
    
    os->printAt(1, 10, STRING("PRESSURE  (hPa):"));
    itoa(456, str_buf);
    printAt(18, 10, str_buf);

}

// ============================================================================
//   APPLICATION MAIN ENTRYPOINT
// ============================================================================

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    os = api;

    os->clear();

    os->pinMode(SDA_PIN, INPUT_PULLUP);
    os->pinMode(SCL_PIN, INPUT_PULLUP);
    os->delay(100);

    uint8_t chip_id = 0;
    bool connected = sensor_read_bytes(REG_CHIP_ID, &chip_id, 1);

    if (connected && chip_id == 0x61) {
        os->println(STRING("[Found] BME68x Chip Found\n"));
        
        // --------------------------------------------------------------------
        // Read Calibration Parameters (Bosch BME68x Memory Layout)
        // --------------------------------------------------------------------
        uint8_t calib_1[25] ALIGNED;
        uint8_t calib_2[16] ALIGNED;
        
        // BME68x calibration is divided into two distinct regions
        sensor_read_bytes(0x89, calib_1, 25);
        sensor_read_bytes(0xE1, calib_2, 16);
        
        // Map Temperature calibration factors
        dig_T1 = (calib_2[2] << 8) | calib_2[1];
        dig_T2 = (calib_1[2] << 8) | calib_1[1];
        dig_T3 = calib_1[3]; 

        // Map Pressure calibration factors
        dig_P1 = (calib_1[6] << 8) | calib_1[5];
        dig_P2 = (calib_1[8] << 8) | calib_1[7];
        dig_P3 = calib_1[9];
        dig_P4 = (calib_1[12] << 8) | calib_1[11];
        dig_P5 = (calib_1[14] << 8) | calib_1[13];
        dig_P6 = calib_1[15];
        dig_P7 = calib_1[16];
        dig_P8 = (calib_1[19] << 8) | calib_1[18];
        dig_P9 = (calib_1[21] << 8) | calib_1[20];

        // --------------------------------------------------------------------
        // Configure Sensor Settings & Mode (Forcing Forced Mode)
        // --------------------------------------------------------------------

        // Set Humidity Oversampling to x1 (Register 0x72)
        sensor_write_reg(REG_CTRL_HUM, 0x01);
        
        // Set Temp & Press Oversampling to x1 and trigger "Forced Mode" (0x01)
        // Bits [7:5] = Temperature x1 (0x01)
        // Bits [4:2] = Pressure x1    (0x01)
        // Bits [1:0] = Mode: Forced   (0x01)
        // Binary: 001 001 01 = 0x25
        sensor_write_reg(REG_CTRL_MEAS, 0x25);

    } else {
        os->println(STRING("[Error] No BME68x module detected"));
    }

    os->delay(2000);
    os->clear();

    TouchState touch;
    int loop_counter = 0;
    float current_t = 0.0f, current_p = 0.0f;

    while (1) {
        if (loop_counter % 50 == 0 && connected) {
            uint8_t raw_data[6];
            if (sensor_read_bytes(REG_DATA_START, raw_data, 6)) {
                int32_t adc_P = ((int32_t)raw_data[0] << 12) | ((int32_t)raw_data[1] << 4) | (raw_data[2] >> 4);
                int32_t adc_T = ((int32_t)raw_data[3] << 12) | ((int32_t)raw_data[4] << 4) | (raw_data[5] >> 4);
                
                current_t = adc_T; //compensate_temp(adc_T);
                current_p = adc_P; //compensate_press(adc_P);
            }
        }

        draw_dashboard(current_t, current_p, connected);

        os->getTouch(&touch);
        if (touch.isPressed) {
            if (TOUCH_IN_BOUNDS(touch, 20, os->termHeight - 35, 80, 25)) {
                accent_color = GREEN;
                os->beep(880, 50);
            }
            else if (TOUCH_IN_BOUNDS(touch, 120, os->termHeight - 35, 80, 25)) {
                accent_color = CYAN;
                os->beep(1000, 50);
            }
            else if (TOUCH_IN_BOUNDS(touch, 220, os->termHeight - 35, 80, 25)) {
                accent_color = ORANGE;
                os->beep(1200, 50);
            }
        }

        int key = os->inkey();
        if (key == '\x13' || key == 'Q') {
            break;
        }
        os->delay(20);
        loop_counter++;
    }
    os->clear();
    return 0;
}
