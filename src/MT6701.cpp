#include <Wire.h>
#include "MT6701.hpp"

/**
 * @brief Constructs an MT6701 encoder object.
 *
 * @param device_address I2C address of the MT6701.
 * @param update_interval Interval in milliseconds at which to update the encoder count.
 * @param rpm_threshold RPM threshold for filtering outliers.
 * @param rpm_filter_size Size of the RPM moving average filter.
 */
MT6701::MT6701(uint8_t device_address, int update_interval, int rpm_threshold, int rpm_filter_size)
    : address(device_address),
      updateIntervalMillis(update_interval),
      lastUpdateTime(0),
      count(0),
      accumulator(0),
      rpm(0),
      rpmFilterIndex(0),
      rpmFilterSize(rpm_filter_size > RPM_FILTER_SIZE ? RPM_FILTER_SIZE : rpm_filter_size),
      rpmThreshold(rpm_threshold)
{
    // Initialize the RPM filter array with zeros
    for (int i = 0; i < RPM_FILTER_SIZE; i++) {
        rpmFilter[i] = 0.0f;
    }
}

/**
 * @brief Initializes the MT6701 encoder.
 * @note This function must be called before any other MT6701 functions.
 */
void MT6701::begin()
{
    Wire.begin();
    Wire.setClock(100000); // Lower clock speed for AVR compatibility
}

/**
 * @brief Returns the shaft angle of the encoder in radians.
 *
 * @return Angle in radians within the range [0, 2*PI).
 */
float MT6701::getAngleRadians()
{
    return count * COUNTS_TO_RADIANS;
}

/**
 * @brief Returns the shaft angle of the encoder in degrees.
 *
 * @return Angle in degrees within the range [0, 360).
 */
float MT6701::getAngleDegrees()
{
    return float(count) * COUNTS_TO_DEGREES;
}

/**
 * @brief Returns the accumulated number of full turns of the encoder shaft since initialization.
 *
 * @return Number of full turns.
 */
int MT6701::getFullTurns()
{
    return accumulator / COUNTS_PER_REVOLUTION;
}

/**
 * @brief Returns the accumulated number of turns of the encoder shaft since initialization as a float.
 *
 * @return Number of turns.
 */
float MT6701::getTurns()
{
    return (float)accumulator / float(COUNTS_PER_REVOLUTION);
}

/**
 * @brief Returns the accumulated count the encoder has generated since initialization.
 *
 * @return Raw accumulator value.
 */
int MT6701::getAccumulator()
{
    return accumulator;
}

/**
 * @brief Returns the current RPM of the encoder shaft averaged over 'rpmFilterSize' samples.
 *
 * @return RPM.
 */
float MT6701::getRPM()
{
    float sum = 0;
    for (int i = 0; i < rpmFilterSize; i++) {
        sum += rpmFilter[i];
    }
    return sum / rpmFilterSize;
}

/**
 * @brief Returns the current encoder count.
 *
 * @return Raw count value.
 */
int MT6701::getCount()
{
    return count;
}

/**
 * @brief Updates the encoder count.
 * @note This function must be called regularly in the main loop for AVR architectures.
 */
void MT6701::updateCount()
{
    int newCount = readCount();
    // give it four tries
    for (int i = 0; i < 3 && newCount < 0; i++) {
        newCount = readCount();
    }
    
    if (newCount < 0) {
        return;
    }
    
    int diff = newCount - count;
    if (diff > COUNTS_PER_REVOLUTION / 2) {
        diff -= COUNTS_PER_REVOLUTION;
    }
    else if (diff < -COUNTS_PER_REVOLUTION / 2) {
        diff += COUNTS_PER_REVOLUTION;
    }
    
    unsigned long currentTime = millis();
    unsigned long timeElapsed = currentTime - lastUpdateTime;
    
    if (timeElapsed > 0) {
        // Calculate RPM
        rpm = (diff / (float)COUNTS_PER_REVOLUTION) * (SECONDS_PER_MINUTE * 1000 / (float)timeElapsed);
        if (abs(rpm) < rpmThreshold) {
            updateRPMFilter(rpm);
        }
    }
    
    accumulator += diff;
    count = newCount;
    lastUpdateTime = currentTime;
}

void MT6701::updateRPMFilter(float newRPM)
{
    rpmFilter[rpmFilterIndex] = newRPM;
    rpmFilterIndex = (rpmFilterIndex + 1) % rpmFilterSize;
}

int MT6701::readCount()
{
    Wire.beginTransmission(address);
    Wire.write(0x03);                  // Starting register ANGLE_H
    Wire.endTransmission(false);       // End transmission, but keep the I2C bus active
    Wire.requestFrom((int)address, 2); // Request two bytes
    
    if (Wire.available() < 2) {
        return -1;
    }
    
    int angle_h = Wire.read();
    int angle_l = Wire.read();

    return (angle_h << 6) | (angle_l >> 2); // returns value from 0 to 16383
}