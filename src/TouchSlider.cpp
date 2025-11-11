/****
 * This file is a part of the TouchSlider Arduino library for AVR architecture MPUs. See TouchSLider.h for 
 * details.
 * 
 *****
 * 
 * TouchSlider V1.0.2, November 2025
 * Copyright (C) 2025 D.L. Ehnebuske
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

// public member functions

TouchSlider::TouchSlider(uint8_t p[], uint8_t pCount) {
    if (pCount < 2 || pCount > MAX_SENSORS) {
        nSensors = 0;
        return;
    }
    nSensors = pCount;
    for (uint8_t s = 0; s < pCount; s++) {
        new (&sensor[s]) TouchSensor(p[s]);     // Use "placement new" to instantiate TouchSensors
        sensorPin[s] = p[s];
    }
}

bool TouchSlider::begin(int32_t minV, int32_t maxV, int32_t curV, int32_t inc) {
    if (nSensors < 2) {
        return false;
    }
    minValue = minV;
    maxValue = maxV;
    value = curV;
    increment = inc;

    for (uint8_t s = 0; s < nSensors; s++) {
        if (!sensor[s].begin()) {
            for (uint8_t ss = 0; ss <= s; ss++) {
                sensor[ss].end();
            }
            return false;
        }
        sensor[s].setTouchedHandler(touchedThunk, this);
        sensor[s].setReleasedHandler(releasedThunk, this);
    }
    inService = true;
    return true;
}

bool TouchSlider::begin() {
    return begin(MIN_MIN_32, MAX_MAX_32);
}

void TouchSlider::end() {
    if (!inService || nSensors < 2) {
        return;
    }
    for (uint8_t s= 0; s < nSensors; s++) {
        sensor[s].end();
    }
    inService = false;
}

TouchSlider::~TouchSlider() {
    if (nSensors < 2) {
        return;
    }

    end();

    for (uint8_t s = 0; s < nSensors; s++) {
        sensor[s].~TouchSensor();
    }
}

void TouchSlider::setChangeHandler(tsl_handler_t handler, void* client) {
    changeHandler = handler;
    clientData = client;
}

int32_t TouchSlider::getValue() {
    return value;
}

#ifdef TSL_DEBUG
void TouchSlider::printState() {
    for (uint8_t s = 0; s < nSensors; s++) {
        Serial.print(sensorTouched[s] ? F("T ") : F("n "));
    }
}
#endif

// private member functions

void TouchSlider::touchedThunk(uint8_t pin, void* client) {
    auto* instance = static_cast<TouchSlider*>(client);
    instance->onTouched(pin);
}

void TouchSlider::onTouched(uint8_t pin) {
    uint8_t sensorS = NUM_DIGITAL_PINS;
    for (uint8_t sNo = 0; sNo < nSensors; sNo++) {
        if (pin == sensorPin[sNo]) {
            sensorS = sNo;
            break;
        }
    }
    uint8_t sensorPrev = sensorS == 0 ? nSensors - 1 : sensorS - 1;
    bool nowTouchedPrev = sensor[sensorPrev].beingTouched();
    bool wasTouchedPrev = sensorTouched[sensorPrev];

    sensorTouched[sensorS] = true;
    sensorTouched[sensorPrev] = nowTouchedPrev;

    int64_t inc = wasTouchedPrev && nowTouchedPrev ? increment : 0;
    
    // Return if no slide
    if (inc == 0) {
        return;
    }

    int64_t newValue = (int64_t)value + inc;
    newValue = newValue > maxValue ? maxValue : newValue < minValue ? minValue : newValue;
    if (newValue != value && changeHandler) {
        changeHandler(newValue, clientData);
    }
    value = newValue;
}

void TouchSlider::releasedThunk(uint8_t pin, void* client) {
    auto* instance = static_cast<TouchSlider*>(client);
    instance->onReleased(pin);
}

void TouchSlider::onReleased(uint8_t pin) {
    uint8_t sensorS = NUM_DIGITAL_PINS;
    for (uint8_t sNo = 0; sNo < nSensors; sNo++) {
        if (pin == sensorPin[sNo]) {
            sensorS = sNo;
            break;
        }
    }
    uint8_t sensorPrev = sensorS == 0 ? nSensors - 1 : sensorS - 1;
    bool nowTouchedPrev = sensor[sensorPrev].beingTouched();
    bool wasTouchedPrev = sensorTouched[sensorPrev];

    sensorTouched[sensorS] = false;
    sensorTouched[sensorPrev] = nowTouchedPrev;

    int64_t inc = wasTouchedPrev && nowTouchedPrev ? -increment : 0;
    
    // Return if no slide
    if (inc == 0) {
        return;
    }

    int64_t newValue = (int64_t)value + inc;
    newValue = newValue > maxValue ? maxValue : newValue < minValue ? minValue : newValue;
    if (newValue != value && changeHandler) {
        changeHandler(newValue, clientData);
    }
    value = newValue;
}