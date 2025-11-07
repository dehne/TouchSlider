/****
 * @file    main.cpp
 * @author  D. L. Ehnebuske (dle.com@ehnebuske.net)
 * @brief   Experiment to see if I can implement a capacitive touch sensor in a really simple way.
 * @version 0.1
 * @date    2025-09-09
 * 
 ****
 * Copyright (C) 2024-2025 D. L. Ehnebuske
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 * 
 ****/
#include <Arduino.h>
#include <TouchSensor.h>
#include <TouchSlider.h>

#define BANNER          F("\nCapTouch 0.2")
#define INIT_MILLIS     (2000)                            // millis() to delay at startup
#define SENSOR_COUNT    (4)                               // The number of sensors we have
#define SENSOR_A_PIN    (2)                               // GPIO to which sensor "A" is attached
#define SENSOR_B_PIN    (3)                               // GPIO to which sensor "B" is attached
#define SENSOR_C_PIN    (4)                               // GPIO to which sensor "C" is attached
#define SENSOR_D_PIN    (5)                               // GPIO to which sensor "D" is attached

constexpr uint32_t SLIDER_MIN = -100;                     // The lowest the slider can be
constexpr uint32_t SLIDER_MAX = 100;                      // The highest the slider can be

#define DEBUG                                             // Uncomment to enable debugging code

/**
 * @brief Hacky convenience function to convert a sensor index into its name.
 * 
 * @return char 
 */
inline char sensorName(uint8_t i) {
  return (char)('A' + i);  
}

uint8_t pins[] = {SENSOR_A_PIN, SENSOR_B_PIN, SENSOR_C_PIN, SENSOR_D_PIN};
TouchSlider slider {pins, SENSOR_COUNT};

/**
 * @brief   Our "change handler." Called (from run()) when a change in the value of our slider is detected.
 * 
 * @param value 
 */
void onChanged(int32_t value) {
  Serial.print(F("\r"));
  slider.printState();
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
  slider.setChangeHandler(onChanged);
}

void loop() {
  // Let the sensors do their thing
  TouchSensor::run();
  slider.run();
  }
