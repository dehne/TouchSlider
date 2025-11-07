/****
 * This file is a part of the TouchSlider Arduino library for AVR architecture MPUs. It gives Arduino sketches a 
 * simple-to-use interface to self-capacitance capacitive touch sliders. TouchSlider builds on, and can be used 
 * in conjunction with, the TouchSensor library. 
 * 
 * A touch slider is a linear or circular series of self-capacitance touch sensors meant to operate as a single 
 * control that has an adjustable numerical value. To use one you slide a finger along (or around) the series of 
 * sensors, in one direction to raise the value or in the opposite direction to lower it. 
 * 
 * There are many ways to construct the individual sensors. For experimenting, you can construct them from small 
 * (~225mm**2) adjacent areas of PCB with copper on one side and covered with thin plastic -- think packing tape. 
 * A signal lead is connected directly to each piece of copper; a ground lead is also connected to each piece of 
 * copper, but through ~2 megohm resistors. The signal leads are connected to digital GPIO pins. The ground leads 
 * attach to, well, signal ground. A more finished sensor can be made from copper pads designed into a PCB with 
 * signal and ground traces connected as above. Like "experimental" sensors, it's important that the copper areas 
 * are covered with a thin dielectric of some sort -- maybe solder mask or plastic tape -- so that the copper is 
 * not touched directly.
 * 
 * Microchip's Capacitive Touch Sensor Design Guide:
 * 
 *      https://ww1.microchip.com/downloads/en/AppNotes/Capacitive-Touch-Sensor-Design-Guide-DS00002934-B.pdf
 * 
 * is a good starting point for information on designing and building capacitive touch sensors of various sorts.
 * 
 * Because of how things work, you can't use TouchSlider (or TouchSensor for that matter) and the Arduino tone() 
 * function in the same sketch. 
 * 
 * How to Use the Library
 * ======================
 * 
 * The intended usage pattern is as follows.
 * 
 * Create a series of touch sensors in a linear or circular array as outlined above and hook them up to GPIO pins 
 * and ground. Instantiate your TouchSlider, passing it an array containing the list of GPIO pins you've used and 
 * the pin-count. It's important that the order of the pins passed matches their physical order. A finger swipe 
 * from the sensor that's first in the list to the last will raise the TouchSlider's value. Often, a TouchSlider 
 * is declared as a global variable.
 * 
 * Next, typically in setup(), initialize the TouchSlider by calling its begin() member function. Here you can 
 * specify the maximum and minimum values the TouchSlider can be set to, together with its initial value, the 
 * increment by which it steps and the factor by which the increment is to be multiplied when a fast finger slide 
 * is detected. 
 * 
 * Because TouchSlider is built on TouchSensor, you'll need to call TouchSensor::run() in loop(). Each call 
 * updates the state of all the TouchSensors that make up the TouchSlider, so call it a lot. In addition, to 
 * update the state of the TouchSlider itself, call its run() member function. Usually, this is done right after 
 * calling TouchSensor::run().
 * 
 * At any point, you can query the current value of your TouchSlider by calling its getValue() member function.
 * 
 * Alternatively (or in addition) you can call the setChangeHandler() member function to register an on-change 
 * callback function. Once you do this, the function you registered will be called (from within run()) whenever 
 * the value of the TouchSlider changes. Typically, registering an on-change callback is done in setup().
 * 
 * If you don't need a TouchSlider for a while, call its end() member function. This will cause its value to 
 * stop changing and, with no value changes, there will be no on-change callbacks made. If you need the 
 * TouchSlider again, call begin(). Value changes and on-change callbacks will resume.
 * 
 * If you no longer require your TouchSlider at all, call its dtor. 
 * 
 * How It Works
 * ============
 * 
 * The TouchSlider relies on the TouchSensor library. It instantiates a TouchSensor for each pin that it's 
 * passed in its ctor. It uses patterns in the changing state of its TouchSensors to keep track of how sliding 
 * touches happen and adjusts the TouchSlider's value accordingly. 
 * 
 * More specifically, let c be the number of sensors the TouchSlider has. Let S(i) be the state pair seen by the 
 * TouchSlider's i-th sensor at the time run() was last called and at the current time. The possible values for 
 * S(i) are: 
 * 
 *      n-->n (not touched last time run() was called, and still not touched this time), 
 *      n-->t (not touched last time, touched this time), 
 *      t-->n (touched last time, not this time) and 
 *      t-->t (touched both times).
 * 
 * Then, with the constraint 0 <= i < c, if we see the pattern
 * 
 *      S((i + 1) mod c) = n-->t & S(i) = t-->t we increment the value of the TouchSlider by inc, 
 *      S((i + 1) mod c) = t-->t & S(i) = n-->t we decrement the value of the TouchSlider by inc,  
 *      S((i + 1) mod c) = n-->t & S(i) = t-->n we increment the value of the TouchSlider by f * inc and 
 *      S((i + 1) mod c) = t-->n & S(i) = n-->t we decrement the value of the TouchSLider by f * inc.
 * 
 * All other patterns have no effect on the value of the TouchSlider.
 * 
 * Here inc is the increment and f is fastFactor specified for the TouchSlider.
 * 
 * All of this incrementing and decrementing is subject, of course, to the constraint that 
 * minValue <= value <= maxValue.
 * 
 *****
 * 
 * TouchSlider V0.1.0, November 2025
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
#pragma once
#ifndef Arduino_h
    #include <Arduino.h>                                // Arduino goop
#endif
#ifndef TouchSensor_h
    #include <TouchSensor.h>                            // TouchSensor goop
#endif

constexpr int32_t MAX_MAX_32 = 0x7FFFFFFF;              // The biggest 32-bit signed integer
constexpr int32_t MIN_MIN_32 = 0x80000000;              // The smallest 32-bit signed integer
constexpr uint8_t MAX_SENSORS = 6;                      // The maximum number of sensors we might have
                                                        //   Can be set to as many as NUM_DIGITAL_PINS
constexpr int32_t QUICK_MOVE_FACTOR = 2;                // Multiply "increment" by this when a finger move is quick

class TouchSlider {
public:
    /**
     * @brief Construct a new Touch Slider object
     * 
     * @param p         The array of GPIO pin numbers to which the TouchSensors making up this TouchSlider are
     *                  attached, in order from the low value direction to the high value direction.
     * @param pCount    The number of pins in p. p >= 2
     */
    TouchSlider(uint8_t p[], uint8_t pCount);

    /**
     * @brief   Put the TouchSlider into service
     * 
     * @param minV      The minimum value the TouchSlider can be set to. Trying to go "down" from this has no effect
     * @param maxV      The maximum value the TouchSlider can be set to. Trying to go "up" from this has no effect. 
     *                  maxValue must be more than minValue.
     * @param curV      The current (initial) value of the TouchSlider. minValue <= curValue <= maxValue.
     * @param inc       The smallest increment by which the TouchSlider's value can change. inc must be more than 0.
     * @param fFactor   The factor for inc when a speedy slide is detected
     * @return true     The TouchSlider was successfully started
     * @return false    The TouchSlider was not successfully started
     */
    bool begin(int32_t minV, int32_t maxV, int32_t curV = 0, int32_t inc = 1, int32_t fFactor = QUICK_MOVE_FACTOR);
    
    /**
     * @brief   Put the TouchSlider into service with default values. Equivalent to 
     *          begin(MIN_MIN_32, 0, MAX_MAX_32, 1);
     * 
     * @return true 
     * @return false 
     */
    bool begin();

    /**
     * @brief   Let the TouchSlider do its thing. Call this often (usually from insiode loop()) to keep the 
     *          slider up to date with respect to what's happening in the real world.
     * 
     */
    void run();

    /**
     * @brief   Take the TouchSlider out of service. A TouchSlider taken out of service can be put back into 
     *          service by calling begin().
     * 
     */
    void end();


    /**
     * @brief Destroy the Touch Slider object, freeing up all its resources
     * 
     */
    ~TouchSlider();

    /**
     * @brief   The type a client-provided "slider change handler" function must have. Write a function with this 
     *          shape, register it with setChangeHandler(), and it'll be called (from run()) whenever the 
     *          slider's value gets changed. The new value of the slider is passed as the parameter.
     * 
     */
    using tsl_handler_t = void (*)(int32_t sliderValue);

    void setChangeHandler(tsl_handler_t handler);

    void printState();
    
private:
    tsl_handler_t changeHandler = nullptr;              // The client-provided value-change handler, if any
    int32_t minValue;                                   // The minimum value the TouchSlide can take on
    int32_t maxValue;                                   // The maximum value the TouchSLider can take on
    int32_t value;                                      // The current value of the TouchSlider
    int32_t increment;                                  // The smallest increment the TouchSlider can change by
    int32_t fastFactor;                                 // Multiply increment by this on fast slides
    alignas(TouchSensor) unsigned char sensorStg[MAX_SENSORS * sizeof(TouchSensor)];
                                                        // Storage to instantiate our TouchSensors
    TouchSensor* sensor = reinterpret_cast<TouchSensor *>(sensorStg);
                                                        // Reinterpreted as TouchSensors for convenience
    uint8_t nSensors;                                   // How many TouchSensors we have
    bool sensorTouched[MAX_SENSORS] = { false };        // The state of the sensors (touched or not) at last run()
    bool inService = false;                             // True if the TpuchSlider is in service, false otherwise
};