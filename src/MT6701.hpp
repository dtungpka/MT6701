#pragma once
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

class MT6701
{
public:
    static constexpr uint8_t DEFAULT_ADDRESS = 0b0000110; // I2C address of the MT6701
    static constexpr int UPDATE_INTERVAL = 50;            // Update interval in milliseconds
    static constexpr int COUNTS_PER_REVOLUTION = 16384;   // 14 bit encoder
    static constexpr float COUNTS_TO_RADIANS = 2.0 * PI / COUNTS_PER_REVOLUTION;
    static constexpr float COUNTS_TO_DEGREES = 360.0 / COUNTS_PER_REVOLUTION;
    static constexpr float SECONDS_PER_MINUTE = 60.0f;
    static constexpr int RPM_THRESHOLD = 1000;            // RPM threshold for filtering
    static constexpr int RPM_FILTER_SIZE = 10;            // Reduced filter size for AVR

    MT6701(uint8_t device_address = DEFAULT_ADDRESS,
           int update_interval = UPDATE_INTERVAL,
           int rpm_threshold = RPM_THRESHOLD,
           int rpm_filter_size = RPM_FILTER_SIZE);
    
    void begin();
    float getAngleRadians();
    float getAngleDegrees();
    int getFullTurns();
    float getTurns();
    int getAccumulator();
    int getCount();
    float getRPM();
    void updateCount();

private:
    uint8_t address;
    int updateIntervalMillis;
    unsigned long lastUpdateTime;
    int count;
    int accumulator;
    float rpm;
    
    // Simple circular buffer for RPM values instead of vector
    float rpmFilter[RPM_FILTER_SIZE];
    int rpmFilterIndex;
    int rpmFilterSize;
    int rpmThreshold;

    int readCount();
    void updateRPMFilter(float newRPM);
};