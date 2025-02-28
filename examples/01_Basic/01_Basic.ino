#include "MT6701.hpp"

MT6701 mt6701;

void setup()
{
    Serial.begin(9600);  // Lower baud rate for AVR boards
    mt6701.begin();
}

void loop()
{
    // Must call updateCount in every loop for AVR architectures
    mt6701.updateCount();
    
    Serial.print("Angle (radians): ");
    Serial.println(mt6701.getAngleRadians());
    Serial.print("Angle (degrees): ");
    Serial.println(mt6701.getAngleDegrees());
    Serial.println();
    delay(100);  // Update more frequently for proper tracking
}