# The TouchSlider Library

## Introduction

The TouchSlider Arduino library for AVR architecture MPUs gives Arduino sketches a simple-to-use interface to self-capacitance capacitive touch sliders. TouchSlider builds on, and can be used in conjunction with, the TouchSensor library.

A touch slider is a linear or circular series of self-capacitance touch sensors meant to operate as a single control that has an adjustable numerical value. To use one you slide a finger along (or around) the series of sensors, in one direction to raise the value or in the opposite direction to lower it.

There are many ways to construct the individual sensors. For experimenting, you can construct them from small (~225mm**2) adjacent areas of PCB with copper on one side and covered with thin plastic -- think packing tape. A signal lead is connected directly to each piece of copper; a ground lead is also connected to each piece of copper, but through ~2 megohm resistors. The signal leads are connected to digital GPIO pins. The ground leads attach to, well, signal ground. A more finished sensor can be made from copper pads designed into a PCB with signal and ground traces connected as above. Like "experimental" sensors, it's important that the copper areas are covered with a thin dielectric of some sort -- maybe solder mask or plastic tape -- so that the copper is not touched directly.

[Microchip's Capacitive Touch Sensor Design Guide](https://ww1.microchip.com/downloads/en/AppNotes/Capacitive-Touch-Sensor-Design-Guide-DS00002934-B.pdf) is a good starting point for information on designing and building capacitive touch sensors of various sorts.

Because of how things work, you can't use TouchSlider (or TouchSensor for that matter) and the Arduino tone() 
function in the same sketch.

## How to Use the Library

The intended usage pattern is as follows.

Create a series of touch sensors in a linear or circular array as outlined above and hook them up to GPIO pins and ground. Instantiate your TouchSlider, passing it an array containing the list of GPIO pins you've used and the pin-count. It's important that the order of the pins passed matches their physical order. A finger swipe from the sensor that's first in the list to the last will raise the TouchSlider's value. Often, a TouchSlider is declared as a global variable.

Next, typically in setup(), initialize the TouchSlider by calling its begin() member function. Here you can specify the maximum and minimum values the TouchSlider can be set to, together with its initial value, the increment by which it steps and the factor by which the increment is to be multiplied when a fast finger slide is detected.

Because TouchSlider is built on TouchSensor, you'll need to call TouchSensor::run() in loop(). Each call updates the state of all the TouchSensors that make up the TouchSlider, so call it a lot. In addition, to update the state of the TouchSlider itself, call its run() member function. Usually, this is done right after calling TouchSensor::run().

At any point, you can query the current value of your TouchSlider by calling its getValue() member function.

Alternatively (or in addition) you can call the setChangeHandler() member function to register an on-change callback function. Once you do this, the function you registered will be called (from within run()) whenever the value of the TouchSlider changes. Typically, registering an on-change callback is done in setup().

If you don't need a TouchSlider for a while, call its end() member function. This will cause its value to stop changing and, with no value changes, there will be no on-change callbacks made. If you need the TouchSlider again, call begin(). Value changes and on-change callbacks will resume.

If you no longer require your TouchSlider at all, call its dtor.

## How It Works

The TouchSlider relies on the TouchSensor library. It instantiates a TouchSensor for each pin that it's passed in its ctor. It uses patterns in the changing state of its TouchSensors to keep track of how sliding touches happen and adjusts the TouchSlider's value accordingly.

More specifically, let c be the number of sensors the TouchSlider has. Let S(i) be the state pair seen by the TouchSlider's i-th sensor at the time run() was last called and at the current time. The possible values for S(i) are:

&nbsp;&nbsp;&nbsp;&nbsp;n-->n (not touched last time run() was called, and still not touched this time),

&nbsp;&nbsp;&nbsp;&nbsp;n-->t (not touched last time, touched this time),

&nbsp;&nbsp;&nbsp;&nbsp;t-->n (touched last time, not this time) and

&nbsp;&nbsp;&nbsp;&nbsp;t-->t (touched both times).

Then, with the constraint 0 <= i < c, if we see the pattern

&nbsp;&nbsp;&nbsp;&nbsp;S((i + 1) mod c) = n-->t & S(i) = t-->t we increment the value of the TouchSlider by inc,

&nbsp;&nbsp;&nbsp;&nbsp;S((i + 1) mod c) = t-->t & S(i) = n-->t we decrement the value of the TouchSlider by inc,  

&nbsp;&nbsp;&nbsp;&nbsp;S((i + 1) mod c) = n-->t & S(i) = t-->n we increment the value of the TouchSlider by q * inc and

&nbsp;&nbsp;&nbsp;&nbsp;S((i + 1) mod c) = t-->n & S(i) = n-->t we decrement the value of the TouchSLider by q * inc.

All other patterns have no effect on the value of the TouchSlider.

Here inc is the increment and f is fastFactor specified for the TouchSlider.

All of this incrementing and decrementing is subject, of course, to the constraint that minValue <= value <= maxValue.
