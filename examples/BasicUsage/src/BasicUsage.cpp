/****
 * @file    BasicUsage.cpp
 * @author  D. L. Ehnebuske (dle.com@ehnebuske.net)
 * @brief   Experiment to see if I can implement a capacitive touch sensor in a really simple way.
 * @version 1.0.0
 * @date    2025-11-09
 * 
 ****
 * Copyright (C) 2025 D. L. Ehnebuske
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
 * documentation files (the "Software"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
 * Software, and to permit persons to whom the Software is furnished to do so, subject to the following 
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions 
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED 
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE. 
 * 
 ****/
#include <Arduino.h>
#include <TouchSensor.h>
#include <TouchSlider.h>

#define BANNER          F("\nTouchSlider Basic Example V1.0.0")
constexpr unsigned long INIT_MILLIS =   2000;             // millis() to delay at startup
constexpr uint8_t       SENSOR_COUNT =  4;                // The number of sensors we have
constexpr uint8_t       SENSOR_A_PIN =  2;                // GPIO to which sensor "A" is attached
constexpr uint8_t       SENSOR_B_PIN =  3;                // GPIO to which sensor "B" is attached
constexpr uint8_t       SENSOR_C_PIN =  4;                // GPIO to which sensor "C" is attached
constexpr uint8_t       SENSOR_D_PIN =  5;                // GPIO to which sensor "D" is attached
constexpr int32_t       SLIDER_MIN =    -100;             // The lowest the slider can be
constexpr int32_t       SLIDER_MAX =    100;              // The highest the slider can be

uint8_t pins[] = {SENSOR_A_PIN, SENSOR_B_PIN, SENSOR_C_PIN, SENSOR_D_PIN};
TouchSlider slider {pins, SENSOR_COUNT};

/**
 * @brief   Our "change handler." Called by slider when a change in its value is detected.
 * 
 * @param value   The new value of the TouchSlider
 * @param notUsed Unused parameter containing whever it was we passed when the change handler was registered
 *                in our case, it's nullptr.
 */
void onChanged(int32_t value, void* notUsed) {
  Serial.print(F("\r"));
  #ifdef TSL_DEBUG
  slider.printState();
  #endif
  Serial.print(F("Slider: "));
  Serial.print(value);
  Serial.print(F("   "));
}
  
void setup() {
  Serial.begin(9600);
  delay(INIT_MILLIS);
  Serial.println(BANNER);

  if (slider.begin(SLIDER_MIN, SLIDER_MAX)) {
    Serial.println(F("Slider initialized successfully."));
  } else {
    Serial.println(F("Slider failed to initialize"));
    while(true) {
      // Spin!
    }
  }
  slider.setChangeHandler(onChanged, nullptr);
}

void loop() {
  // Let the sensors do their thing
  TouchSensor::run();
  }
