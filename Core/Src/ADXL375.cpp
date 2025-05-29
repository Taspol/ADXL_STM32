/*
 * ADXL375.cpp
 *
 *  Created on: Apr 8, 2024
 *      Author: HP
 *  Converted to OOP: Object-Oriented C++ Version
 */

#include "ADXL375.h"

/*
 * CONSTRUCTORS AND DESTRUCTOR
 */
ADXL375::ADXL375() : spiHandle(nullptr) {
    for (int i = 0; i < 6; i++) {
        rawAccData[i] = 0;
    }
    for (int i = 0; i < 3; i++) {
        accData[i] = 0.0f;
    }
}
ADXL375::ADXL375(SPI_HandleTypeDef *handle) : ADXL375() {
    initialize(handle);
}

ADXL375::~ADXL375() {
    // Clean up if needed
    if (spiHandle != nullptr) {
        toggleCSHigh();
    }
}

/*
 * INITIALIZATION
 */
int ADXL375::initialize(SPI_HandleTypeDef *handle) {
    if (handle == nullptr) {
        return -1; // Invalid handle
    }

    toggleCSHigh();
    spiHandle = handle;

    HAL_StatusTypeDef status;
    int errorNum = 0;

    /* Confirm device address as 0xE5 */
    uint8_t confirmID;
    int attempts = 0;
    const int maxAttempts = 100;

    while (attempts < maxAttempts) {
        status = readData(DEVID, &confirmID, 1);
        if (status == HAL_OK && confirmID == DEVID_WHOAMI) {
            break;
        }
        attempts++;
        HAL_Delay(10);
    }
    if (attempts >= maxAttempts) {
        return -2; // Device not found
    }

    /* Set the  data rate */
    status = writeData(BW_RATE, BW_RATE_DATA_RATE, 1);
    if (status != HAL_OK) {
        errorNum++;
    }

    /* Enable measurement mode */
    status = writeData(POWER_CTL, POWER_CTL_MEASURE_MODE, 1);
    if (status != HAL_OK) {
        errorNum++;
    }

    /* Set data format */
    status = writeData(DATA_FORMAT, POWER_CTL_MEASURE_MODE, 1);
    if (status != HAL_OK) {
        errorNum++;
    }

    return errorNum;
}

bool ADXL375::isInitialized() const {
    return (spiHandle != nullptr);
}

/*
 * DATA ACQUISITION
 */
HAL_StatusTypeDef ADXL375::readAcceleration() {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = readData(DATAX0, rawAccData, 6);

    if (status == HAL_OK) {
        cleanRawValues();
    }

    return status;
}

/*
 * GETTERS
 */
void ADXL375::getAcceleration(float &x, float &y, float &z) const {
    x = accData[0];
    y = accData[1];
    z = accData[2];
}

void ADXL375::getAccelerationArray(float acc[3]) const {
    acc[0] = accData[0];
    acc[1] = accData[1];
    acc[2] = accData[2];
}

/*
 * UTILITY METHODS
 */
bool ADXL375::devicePresent() {
    if (!isInitialized()) {
        return false;
    }

    uint8_t deviceId;
    HAL_StatusTypeDef status = readData(DEVID, &deviceId, 1);

    return (status == HAL_OK && deviceId == DEVID_WHOAMI);
}

HAL_StatusTypeDef ADXL375::setDataRate(uint8_t rate) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(BW_RATE, rate, 1);
}

HAL_StatusTypeDef ADXL375::setPowerMode(bool measureMode) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    uint8_t value = measureMode ? POWER_CTL_MEASURE_MODE : 0x00;
    return writeData(POWER_CTL, value, 1);
}

HAL_StatusTypeDef ADXL375::setDataFormat(uint8_t format) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(DATA_FORMAT, format, 1);
}

/*
 * CONFIGURATION METHODS
 */
HAL_StatusTypeDef ADXL375::setOffsets(int8_t offsetX, int8_t offsetY, int8_t offsetZ) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = writeData(OFFSET_X, static_cast<uint8_t>(offsetX), 1);
    if (status != HAL_OK) return status;

    status = writeData(OFFSET_Y, static_cast<uint8_t>(offsetY), 1);
    if (status != HAL_OK) return status;

    status = writeData(OFFSET_Z, static_cast<uint8_t>(offsetZ), 1);
    return status;
}

HAL_StatusTypeDef ADXL375::setActivityThreshold(uint8_t threshold) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(ACT_THRESH, threshold, 1);
}

HAL_StatusTypeDef ADXL375::setInactivityThreshold(uint8_t threshold) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    return writeData(INACT_THRESH, threshold, 1);
}

/*
 * PRIVATE METHODS
 */
void ADXL375::cleanRawValues() {
    int16_t val;

    /* CONVERSION FOR ACC_X */
    val = static_cast<int16_t>((rawAccData[1] << 8) | rawAccData[0]);
    accData[0] = static_cast<float>(val) * SCALE_FACTOR;

    /* CONVERSION FOR ACC_Y */
    val = static_cast<int16_t>((rawAccData[3] << 8) | rawAccData[2]);
    accData[1] = static_cast<float>(val) * SCALE_FACTOR;

    /* CONVERSION FOR ACC_Z */
    val = static_cast<int16_t>((rawAccData[5] << 8) | rawAccData[4]);
    accData[2] = static_cast<float>(val) * SCALE_FACTOR;
}

HAL_StatusTypeDef ADXL375::writeData(uint8_t address, uint8_t data, uint16_t len) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    toggleCSLow();

    HAL_StatusTypeDef status = HAL_SPI_Transmit(spiHandle, &address, 1, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        status = HAL_SPI_Transmit(spiHandle, &data, len, HAL_MAX_DELAY);
    }

    toggleCSHigh();
    return status;
}

HAL_StatusTypeDef ADXL375::readData(uint8_t address, uint8_t *data, uint16_t len) {
    if (!isInitialized()) {
        return HAL_ERROR;
    }

    toggleCSLow();

    uint8_t txBuffer = (address | 0x80);

    if (len > 1) {
        txBuffer = (txBuffer | 0x40); // Multi-byte read
    }

    HAL_StatusTypeDef status = HAL_SPI_Transmit(spiHandle, &txBuffer, 1, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        status = HAL_SPI_Receive(spiHandle, data, len, HAL_MAX_DELAY);
    }

    toggleCSHigh();
    return status;
}

void ADXL375::toggleCSHigh() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
}

void ADXL375::toggleCSLow() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}
