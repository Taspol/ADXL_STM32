/*
 * ADXL375.h
 *
 *  Created on: Apr 8, 2024
 *      Author: Satvik Agrawal
 *  Converted to OOP: Object-Oriented C++ Version
 */

#ifndef ADXL375_H_
#define ADXL375_H_

#include "stm32f1xx_hal.h" /* Needed for SPI */

class ADXL375 {
private:
    /*
     * REGISTER ADDRESSES
     */
    static const uint8_t DEVID = 0x00;
    static const uint8_t THRESH_SHOCK = 0x1D;
    static const uint8_t OFFSET_X = 0x1E;
    static const uint8_t OFFSET_Y = 0x1F;
    static const uint8_t OFFSET_Z = 0x20;
    static const uint8_t SHOCK_DURATION = 0x21;
    static const uint8_t SHOCK_LATENT = 0x22;
    static const uint8_t SHOCK_WINDOW = 0x23;
    static const uint8_t ACT_THRESH = 0x24;
    static const uint8_t INACT_THRESH = 0x25;
    static const uint8_t TIME_INACT = 0x26;
    static const uint8_t ACT_INACT_CTL = 0x27;
    static const uint8_t SHOCK_AXES = 0x2A;
    static const uint8_t ACT_SHOCK_STAT = 0x2B;
    static const uint8_t BW_RATE = 0x2C;
    static const uint8_t POWER_CTL = 0x2D;
    static const uint8_t INT_ENABLE = 0x2E;
    static const uint8_t INT_MAP = 0x2F;
    static const uint8_t INT_SOURCE = 0x30;
    static const uint8_t DATA_FORMAT = 0x31;
    static const uint8_t DATAX0 = 0x32;
    static const uint8_t DATAX1 = 0x33;
    static const uint8_t DATAY0 = 0x34;
    static const uint8_t DATAY1 = 0x35;
    static const uint8_t DATAZ0 = 0x36;
    static const uint8_t DATAZ1 = 0x37;
    static const uint8_t FIFO_CTL = 0x38;
    static const uint8_t FIFO_STATUS = 0x39;

    /*
     * REGISTER VALUES
     */
    static const uint8_t DEVID_WHOAMI = 0xE5;
    static const uint8_t POWER_CTL_MEASURE_MODE = 0x08;
    static const uint8_t BW_RATE_DATA_RATE = 0x0F;

    /*
     * CONVERSION FACTOR
     */
    static constexpr float SCALE_FACTOR = 49.0f;

    /*
     * MEMBER VARIABLES
     */
    SPI_HandleTypeDef *spiHandle;
    uint8_t rawAccData[6];
    float accData[3];

    /*
     * PRIVATE METHODS - Low-level functions
     */
    HAL_StatusTypeDef writeData(uint8_t address, uint8_t data, uint16_t len);
    HAL_StatusTypeDef readData(uint8_t address, uint8_t *data, uint16_t len);
    void toggleCSHigh();
    void toggleCSLow();
    void cleanRawValues();

public:
    /*
     * CONSTRUCTORS AND DESTRUCTOR
     */
    ADXL375();
    explicit ADXL375(SPI_HandleTypeDef *spiHandle);
    ~ADXL375();

    /*
     * INITIALIZATION
     */
    int initialize(SPI_HandleTypeDef *spiHandle);
    bool isInitialized() const;

    /*
     * DATA ACQUISITION
     */
    HAL_StatusTypeDef readAcceleration();

    /*
     * GETTERS
     */
    float getAccX() const { return accData[0]; }
    float getAccY() const { return accData[1]; }
    float getAccZ() const { return accData[2]; }

    void getAcceleration(float &x, float &y, float &z) const;
    void getAccelerationArray(float acc[3]) const;

    /*
     * UTILITY METHODS
     */
    bool devicePresent();
    HAL_StatusTypeDef setDataRate(uint8_t rate);
    HAL_StatusTypeDef setPowerMode(bool measureMode);
    HAL_StatusTypeDef setDataFormat(uint8_t format);

    /*
     * CONFIGURATION METHODS
     */
    HAL_StatusTypeDef setOffsets(int8_t offsetX, int8_t offsetY, int8_t offsetZ);
    HAL_StatusTypeDef setActivityThreshold(uint8_t threshold);
    HAL_StatusTypeDef setInactivityThreshold(uint8_t threshold);
};

#endif /* ADXL375_H_ */
