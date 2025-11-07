/****
 * This file is a part of the TouchSlider Arduino library for AVR architecture MPUs. See TouchSLider.h for
 * details.
 * 
 *****
 * 
 * TouchSlider V0.1.0, October 2025
 * Copyright (C) 2024-2025 D.L. Ehnebuske
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
#include "TouchSlider.h"
#include <new>

TouchSlider::TouchSlider(uint8_t p[], uint8_t pCount) {
    if (pCount < 2 || pCount > MAX_SENSORS) {
        pCount = 0;
        return;
    }
    nSensors = pCount;
    for (uint8_t s = 0; s < pCount; s++) {
        new (&sensor[s]) TouchSensor(p[s]);     // Use "placement new" to instantiate TouchSensors
    }
}

bool TouchSlider::begin(int32_t minV, int32_t maxV, int32_t curV, int32_t inc, int32_t fFactor) {
    if (nSensors < 2) {
        return false;
    }
    minValue = minV;
    maxValue = maxV;
    value = curV;
    increment = inc;
    fastFactor = fFactor;

    for (uint8_t s = 0; s < nSensors; s++) {
        if (!sensor[s].begin()) {
            for (uint8_t ss = 0; ss <= s; ss++) {
                sensor[ss].end();
            }
            return false;
        }
    }
    return true;
}

bool TouchSlider::begin() {
    return begin(MIN_MIN_32, MAX_MAX_32);
}

void TouchSlider::run() {
    bool newSensorTouched[nSensors];
    for (uint8_t s = 0; s < nSensors; s++) {
    newSensorTouched[s] = sensor[s].beingTouched();                                     // Get new state of all the sensors
    }
    int64_t inc = 0;
    // See if we can find a finger slide
    for (uint8_t s = 0; s < nSensors; s++) {
        uint8_t prevS = s == 0 ? nSensors - 1 : s - 1;
        if (!sensorTouched[s] && newSensorTouched[s] && 
            sensorTouched[prevS] && !newSensorTouched[prevS]) {                         // S(s): n-->t & S(prevS): t-->n
            inc = QUICK_MOVE_FACTOR * increment;                                        // Slid "up" one whole sensor: Up quickly
            break;
        } else if (sensorTouched[s] && !newSensorTouched[s] && 
                    !sensorTouched[prevS] && newSensorTouched[prevS]) {                 // S(s): t-->n & S(prevS): n-->t
            inc = -QUICK_MOVE_FACTOR * increment;                                       // Slid "down" one whole sensor: Down quickly
            break;
        } else if (!sensorTouched[s] && newSensorTouched[s] && 
                    sensorTouched[prevS] && newSensorTouched[prevS]) {                  // S(s): n-->t & S(prevS): t-->t
            inc = increment;                                                            // Slid "up" half a sensor: Up slowly
            break;
        } else if (sensorTouched[s] && newSensorTouched[s] &&
                    !sensorTouched[prevS] && newSensorTouched[prevS]) {                 // S(s): t-->t & S(prevS): n-->t
            inc = -increment;                                                           // Slid "down" half a sensor: Down slowly
            break;
        }
    }
    memcpy(sensorTouched, newSensorTouched, sizeof(newSensorTouched));                  // Update the states of the sensors
    if (inc == 0) {                                                                     // Return if no slide
        return;
    }
    int64_t newValue = (int64_t)value + inc;
    newValue = newValue > maxValue ? maxValue : newValue < minValue ? minValue : newValue;
    if (newValue != value && changeHandler) {
        changeHandler(newValue);
    }
    value = newValue;
}

void TouchSlider::end() {
    if (nSensors < 2) {
        return;
    }
    for (uint8_t s= 0; s < nSensors; s++) {
        sensor[s].end();
    }
}

TouchSlider::~TouchSlider() {
    if (nSensors < 2) {
        return;
    }
    for (uint8_t s = 0; s < nSensors; s++) {
        sensor[s].~TouchSensor();
    }
}

void TouchSlider::setChangeHandler(tsl_handler_t handler) {
    changeHandler = handler;
}

void TouchSlider::printState() {
    for (uint8_t s = 0; s < nSensors; s++) {
        Serial.print(sensorTouched[s] ? F("T ") : F("n "));
    }
}