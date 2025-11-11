/****
 * 
 * Introduction
 * ============
 * 
 * This file is a part of the TouchSlider Arduino library for AVR architecture MPUs. The library gives Arduino 
 * sketches a simple-to-use interface to self-capacitance capacitive touch sliders. TouchSlider builds on, and 
 * can be used in conjunction with, the TouchSensor library.
 * 
 * A touch slider is a linear or circular series of self-capacitance touch sensors meant to operate as a single 
 * control that exposes an adjustable numerical value to your sketch. To use one you slide a finger along (or 
 * around) the series of sensors, in one direction to raise the value or in the opposite direction to lower it.
 * 
 * There are many ways to construct the individual sensors. For experimenting, you can construct them from small 
 * (~225mm**2) adjacent areas of copper on some sort of non-conductive substrate and then covered with thin 
 * plastic -- think copper tape, perfboard and packing tape. Signal and ground leads are connected to each piece 
 * of copper; the signal leads directly and the ground leads through ~2 megohm resistors. The other ends of the 
 * signal leads are connected pairwise to digital GPIO pins. The ground leads all attach to, well, signal ground.
 * 
 * A more finished sensor can be made from copper pads designed into a PCB with signal and ground traces 
 * connected as above. Like "experimental" sensors, it's important that the copper areas are covered with a thin 
 * dielectric of some sort -- maybe solder mask or plastic tape -- so that the copper can't be touched directly.
 * 
 * Microchip's Capacitive Touch Sensor Design Guide
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
 * specify the maximum and minimum values the TouchSlider can be set to, together with its initial value and the 
 * increment by which it steps.
 * 
 * Because TouchSlider is built on TouchSensor, you'll need to call TouchSensor::run() in loop(). Each call 
 * updates the state of all the TouchSensors that make up the TouchSlider, so call it a lot. I've worked hard to 
 * minimize the overhead when nothing's going on, so call it a lot to keep the TouchSlider responsive.
 * 
 * At any point, you can query the current value of your TouchSlider by calling its getValue() member function.
 * 
 * Alternatively (or in addition) you can call the setChangeHandler() member function to register an on-change 
 * callback function. Once you do this, the function you registered will be called whenever the value of the 
 * TouchSlider changes. Typically, registering an on-change callback is done in setup().
 * 
 * If you don't need a TouchSlider for a while, call its end() member function. This will cause its value to stop 
 * changing and, with no value changes, there will be no on-change callbacks made. If you need the TouchSlider 
 * again, call begin(). Value changes and on-change callbacks will resume.
 * 
 * If you no longer require your TouchSlider at all, call its dtor.
 * 
 * How It Works
 * ============
 * 
 * The TouchSlider relies on the TouchSensor library. It instantiates a TouchSensor for each pin that it's passed 
 * in its ctor. It uses patterns in the changing state of its constituent TouchSensors to keep track of how 
 * finger-slides happen and adjusts the TouchSlider's value accordingly.
 * 
 * In particular, whenever the state of one of the TouchSlider's sensors changes, we look at what (if anything) 
 * happened to the state of the sensor that logically precedes it in the array. If, at the time the last state 
 * change occurred for a sensor in the array, the logically preceding sensor was being touched and if it's still 
 * being touched now, we've detected a finger slide. No other condition indicates a slide.
 * 
 * The direction of the slide depends on which state change occurred. A change from not-touched to touched 
 * indicates a slide up, a change from touched to not-touched indicates a slide down.
 * 
 * Think of it this way. If a finger is sliding up, it starts by touching some sensor. As it slides upward it 
 * crosses over into the next sensor, causing that sensor to change state from being not touched to being 
 * touched. So, not-touched to touched and the preceding sensor was being touched and is still being touched. 
 * That's a slide up.
 * 
 * A finger-slide down is a little harder to see, but not too much so. If a finger is sliding down, it's touching 
 * some sensor at the start. As it crosses into the preceding sensor, the crossing causes the preceding sensor to 
 * change from not-touched to touched, but that change is ignored because the sensor preceding the preceding 
 * sensor isn't being touched. As the slide continues, the finger moves to the point where it no longer touches 
 * the sensor where we started this analysis. That causes the sensor where the finger started out to change from 
 * being touched to not being touched. Since its preceding sensor was being touched since the last change 
 * occurred, that's a slide down.
 * 
 * It's worth noting that implicit in this analysis is the idea a finger can't touch more than two sensors at one 
 * time. What if that's not true? Well, the analysis is a bit harder, but things work out. Exercise left to the 
 * reader.
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
#pragma once
#ifndef Arduino_h
    #include <Arduino.h>                                // Arduino goop
#endif
#ifndef TouchSensor_h
    #include <TouchSensor.h>                            // TouchSensor goop
#endif

//#define TSL_DEBUG                                       // Uncomment to enable debugging code

constexpr int32_t MAX_MAX_32 = 0x7FFFFFFF;              // The biggest 32-bit signed integer
constexpr int32_t MIN_MIN_32 = 0x80000000;              // The smallest 32-bit signed integer
constexpr uint8_t MAX_SENSORS = 6;                      // The maximum number of sensors we might have
                                                        //   Can be set to as many as NUM_DIGITAL_PINS

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
     * @param inc       The increment by which the TouchSlider's value can change. inc must be more than 0.
     * @return true     The TouchSlider was successfully started
     * @return false    The TouchSlider was not successfully started
     */
    bool begin(int32_t minV, int32_t maxV, int32_t curV = 0, int32_t inc = 1);
    
    /**
     * @brief   Put the TouchSlider into service with default values. Equivalent to 
     *          begin(MIN_MIN_32, 0, MAX_MAX_32, 1);
     * 
     * @return true 
     * @return false 
     */
    bool begin();

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
     *          shape, register it using setChangeHandler(), and it'll be called whenever the slider's value gets 
     *          changed.
     * 
     * @param   sliderValue The slider's new value.
     * @param   client      The value the client passed when the change handler was registered.
     */
    using tsl_handler_t = void (*)(int32_t sliderValue, void* client);

    /**
     * @brief Set the changeHandler -- the function that will be called when the value of the TouchSlider changes.
     * 
     * @param handler   The function to call
     * @param client    Client provided value. Whatever it is, it will be passed to the function when it's called.
     */
    void setChangeHandler(tsl_handler_t handler, void* client);

    /**
     * @brief Get the current value of the the TouchSlider
     * 
     * @return int32_t  The current value of the TouchSlider
     */
    int32_t getValue();

    #ifdef TSL_DEBUG
    /**
     * @brief Print the current state of the internals of the TouchSlider to Serial for debugging purposes.
     * 
     */
    void printState();
    #endif
    
private:
    static void touchedThunk(uint8_t pin, void* client);    // What we register with TouchSensor as "touched" a callback
    void onTouched(uint8_t pin);                            // The actual callback
    static void releasedThunk(uint8_t pin, void* client);   // What we regoister with TouchSensor as a "released" callback
    void onReleased(uint8_t pin);                           // The actual callback

    tsl_handler_t changeHandler = nullptr;                  // The client-provided value-change handler, if any
    void* clientData;                                       // The client-provided pointer passed to changeHandler
    int32_t minValue;                                       // The minimum value the TouchSlide can take on
    int32_t maxValue;                                       // The maximum value the TouchSLider can take on
    int32_t value;                                          // The current value of the TouchSlider
    int32_t increment;                                      // The increment the TouchSlider can change by
    alignas(TouchSensor) unsigned char sensorStg[MAX_SENSORS * sizeof(TouchSensor)];
                                                            // Storage to instantiate our TouchSensors
    TouchSensor* sensor = reinterpret_cast<TouchSensor *>(sensorStg);
                                                            // Reinterpreted as TouchSensors for convenience
    uint8_t nSensors;                                       // How many TouchSensors we have
    bool sensorTouched[MAX_SENSORS] = { false };            // The state of the sensors (touched or not) at last run()
    uint8_t sensorPin[MAX_SENSORS];                         // The pin number for each of the sensors
    bool inService = false;                                 // True if the TpuchSlider is in service, false otherwise
};