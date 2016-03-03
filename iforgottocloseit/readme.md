This is the Arduino IDE folder for the sketch.

### Blue LED (gpio1) status explanation

When debugging is enabled (**Tools > Debug Port > Serial**) and USB connected, text will be sent and captured in the Arduino IDE Serial Monitor. However when this is disabled, as should be the case when deploying (in your garage), the blue LED (gpio1) can be used to ascertain state of the program.

The basic flow is like this:

* short *blip*s (LED flash) until the wifi connects.
* a long *blah* (1000ms LED flash) when the wifi connects successfully. 

Now the `loop()` starts:

* two short *blip*s every 5 seconds indicates the door is closed
* two long *blah*s indicates the door is open

Note:

* If you power the esp and the blue LED lights and holds on continiously, then the door is closed (gpio1 `LOW`). The program wont start in this state. Open the door and then power esp on again. You will see the *blah blah* indicating the open state. Closing the door should then show *blip blip* at the 5s intival. The system is now ready and waiting for the next open to start the timer and then send the sms.

