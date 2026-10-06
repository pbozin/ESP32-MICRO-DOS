#include "microdos_api.h"
#include "microdos_util.h"

// I2C Pins
#define SDA_PIN 32 
#define SCL_PIN 25

// BME680 / BME688 Registers & Configuration
#define SENSOR_ADDR       0x77
#define REG_CHIP_ID       0xD0
#define REG_CTRL_HUM      0x72
#define REG_CTRL_MEAS     0x74
#define REG_CTRL_GAS_1    0x71
#define REG_DATA_START    0x1D

static int      accent_color = CYAN;
static int32_t  t_fine = 0;

static uint16_t dig_regU[2] ALIGNED = {0, 0};
#define dig_T1 dig_regU[0]
#define dig_P1 dig_regU[1]

static int16_t  dig_regS[11] ALIGNED = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define dig_T2 dig_regS[0]
#define dig_T3 dig_regS[1]
#define dig_P2 dig_regS[2]
#define dig_P3 dig_regS[3]
#define dig_P4 dig_regS[4]
#define dig_P5 dig_regS[5]
#define dig_P6 dig_regS[6]
#define dig_P7 dig_regS[7]
#define dig_P8 dig_regS[8]
#define dig_P9 dig_regS[9]
#define dig_P10 dig_regS[10]

// ============================================================================
//   BIT-BANGED I2C PROTOCOL DRIVER
// ============================================================================

ALWAYS INLINE void i2c_delay() {
    volatile int count = 80;
    while(count--) { __asm__("nop"); }
}

INLINE void i2c_sda_high() {
    pinMode(SDA_PIN, INPUT_PULLUP);
    i2c_delay();
}

INLINE void i2c_sda_low() {
    pinMode(SDA_PIN, OUTPUT);
    digitalWrite(SDA_PIN, 0);
    i2c_delay();
}

INLINE void i2c_scl_high() {
    pinMode(SCL_PIN, INPUT_PULLUP);
    i2c_delay();

    volatile int timeout = 1000;
    while (digitalRead(SCL_PIN) == 0 && timeout > 0) {
        timeout--;
        i2c_delay();
    }
}

INLINE void i2c_scl_low() {
    pinMode(SCL_PIN, OUTPUT);
    digitalWrite(SCL_PIN, 0);
    i2c_delay();
}

// ============================================================================
//   PROTOCOL SIGNALS
// ============================================================================

ALWAYS INLINE void i2c_init_bus() {
    i2c_sda_high();
    i2c_scl_high();
}

ALWAYS INLINE void i2c_start() {
    i2c_sda_high();
    i2c_scl_high();
    i2c_sda_low();
    i2c_scl_low();
}

ALWAYS INLINE void i2c_stop() {
    i2c_sda_low();
    i2c_scl_high();
    i2c_sda_high();
}

ALWAYS INLINE bool i2c_write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (byte & 0x80) i2c_sda_high();
        else i2c_sda_low();
        byte <<= 1;
        i2c_scl_high();
        i2c_scl_low();
    }

    i2c_sda_high();
    i2c_scl_high();

    bool ack ALIGNED = (digitalRead(SDA_PIN) == 0);

    i2c_scl_low();
    return ack;
}

ALWAYS INLINE uint8_t i2c_read_byte(bool send_ack) {
    uint8_t byte ALIGNED = 0;

    i2c_sda_high();

    for (int i = 0; i < 8; i++) {
        i2c_scl_high();
        byte <<= 1;
        if (digitalRead(SDA_PIN)) {
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

ALWAYS INLINE float compensate_temp(int32_t adc_T) {
    int32_t var1 = ((adc_T >> 3) - ((int32_t)dig_T1 << 1));
    int32_t var2 = (var1 * (int32_t)dig_T2) >> 11;
    int32_t var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
    var3 = (var3 * ((int32_t)dig_T3 << 4)) >> 14; 
    
    t_fine = var2 + var3;
    
    int32_t T = (t_fine * 5 + 128) >> 8;
    return (float)T * 0.01f;
}

ALWAYS INLINE float compensate_press(int32_t adc_P) {
    int32_t var1, var2, var3;
    int32_t pressure_comp;

    var1 = (((int32_t)t_fine) >> 1) - 64000;
    var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)dig_P6) >> 2;
    var2 = var2 + ((var1 * (int32_t)dig_P5) << 1);
    var2 = (var2 >> 2) + ((int32_t)dig_P4 << 16);
    
    var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)dig_P3 << 5)) >> 3) + 
           (((int32_t)dig_P2 * var1) >> 1);
    var1 = var1 >> 18;
    var1 = ((32768 + var1) * (int32_t)dig_P1) >> 15;

    if (var1 == 0) return 0.0f;

    pressure_comp = 1048576 - adc_P;
    pressure_comp = (int32_t)((pressure_comp - (var2 >> 12)) * ((uint32_t)3125));

    if (pressure_comp >= 0x40000000L) {
        pressure_comp = ((pressure_comp / var1) << 1);
    } else {
        pressure_comp = ((pressure_comp << 1) / var1);
    }

    var1 = ((int32_t)dig_P9 * (int32_t)(((pressure_comp >> 3) * (pressure_comp >> 3)) >> 13)) >> 12;
    var2 = ((int32_t)dig_P8 * (int32_t)(pressure_comp >> 2)) >> 13;
    var3 = ((int32_t)dig_P10 * (int32_t)(pressure_comp >> 8)) >> 12;
    
    int32_t p = (int32_t)((int32_t)pressure_comp + ((var1 + var2 + var3 + ((int32_t)dig_P7 << 7)) >> 4));
    
    return (float)p * 0.01f;
}

// ============================================================================
//   UI ENGINE & VISUAL DRAWING
// ============================================================================

ALWAYS INLINE void draw_dashboard(float temp, float press, bool sensor_found) {
    static char str_buf[16] ALIGNED;
    termPrintAt(6, 3, STRING("-- BME68X METRICS LAB --"));

    if (!sensor_found) {
        termPrintAt(6, 5, STRING("STATUS: SENSOR NOT FOUND"));
        termPrintAt(6, 6, STRING("Please check SDA and SCL"));
        return;
    }

    termPrintAt(1,  8, STRING("TEMPERATURE (C):"));
    ftoa(temp, str_buf, 16, 3);
    termPrintAt(18, 8, str_buf);
    
    termPrintAt(1, 10, STRING("PRESSURE (mmHg):"));
    ftoa(press, str_buf, 16, 3);
    termPrintAt(18, 10, str_buf);

}

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;

    termClear();

    pinMode(SDA_PIN, INPUT_PULLUP);
    pinMode(SCL_PIN, INPUT_PULLUP);
    delayMs(100);

    uint8_t chip_id = 0;
    bool connected = sensor_read_bytes(REG_CHIP_ID, &chip_id, 1);

    if (connected && chip_id == 0x61) {
        // --------------------------------------------------------
        // Read Calibration Parameters (Bosch BME68x Memory Layout)
        // --------------------------------------------------------
        uint8_t calib_1[25] ALIGNED;
        uint8_t calib_2[16] ALIGNED;
        
        // BME68x calibration is divided into two distinct regions
        sensor_read_bytes(0x89, calib_1, 25);
        sensor_read_bytes(0xE1, calib_2, 16);
        
        // Map Temperature calibration factors
        dig_T1 = (calib_2[2] << 8) | calib_2[1];
        dig_T2 = (calib_1[2] << 8) | calib_1[1];
        dig_T3 = (int8_t)calib_1[3]; 

        dig_P1  = (calib_1[6] << 8) | calib_1[5];
        dig_P2  = (calib_1[8] << 8) | calib_1[7];
        dig_P3  = (int8_t)calib_1[9];  
        dig_P4  = (calib_1[12] << 8) | calib_1[11];
        dig_P5  = (calib_1[14] << 8) | calib_1[13];
        dig_P6  = (int8_t)calib_1[15]; 
        dig_P7  = (int8_t)calib_1[16]; 
        dig_P8  = (calib_1[19] << 8) | calib_1[18];
        dig_P9  = (calib_1[21] << 8) | calib_1[20];
        dig_P10 = (int8_t)calib_2[8]; 

        // --------------------------------------------------------
        // Configure Sensor Settings & Mode (Forcing Forced Mode)
        // --------------------------------------------------------

        // Set Humidity Oversampling to x1 (Register 0x72)
        sensor_write_reg(REG_CTRL_HUM, 0x01);
        
        // Set Temp & Press Oversampling to x1 and trigger "Forced Mode" (0x01)
        // Bits [7:5] = Temperature x1 (0x01)
        // Bits [4:2] = Pressure x1    (0x01)
        // Bits [1:0] = Mode: Forced   (0x01)
        // Binary: 001 001 01 = 0x25
        sensor_write_reg(REG_CTRL_MEAS, 0x25);

        termPrintln(STRING("[Found] BME68x Chip Found\n"));
    } else {
        termPrintln(STRING("[Error] No BME68x module detected"));
    }

    delayMs(2000);
    termClear();

    TouchState touch;
    int loop_counter = 0;
    float current_t = 0.0f, current_p = 0.0f;

    while (1) {
        if (loop_counter % 50 == 0 && connected) {
            sensor_write_reg(REG_CTRL_MEAS, 0x25);
	    delayMs(20);
            uint8_t raw_data[6];
            if (sensor_read_bytes(REG_DATA_START, raw_data, 6)) {
                int32_t adc_P = ((int32_t)raw_data[0] << 12) | ((int32_t)raw_data[1] << 4) | (raw_data[2] >> 4);
                int32_t adc_T = ((int32_t)raw_data[3] << 12) | ((int32_t)raw_data[4] << 4) | (raw_data[5] >> 4);
                
                current_t = compensate_temp(adc_T);
                current_p = compensate_press(adc_P);
            }
        }

        draw_dashboard(current_t, current_p, connected);

        int key = getKey();
        if (key == '\x13' || key == 'Q') {
            break;
        }
        loop_counter++;
        delayMs(20);
    }
    termClear();
    return 0;
}
