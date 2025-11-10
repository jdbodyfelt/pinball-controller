# Pinball Apron Console - for Virtual Pinball Wizards

## System Outline

The console controller is comprised of a number of systems, similar to `/lib` coding structures. 
Here, we focus on the circuitry associated with the device. 

### The Button Array

Let's start with your standard pinball controls. I've extended flipper/magna to a four-button offset stack. 
That should address several user configurations! This yields 8 "side action" buttons. In addition to this, 
there are three front panel buttons - a left "start game" and "next ball", and a right "launch".

All buttons have LED illumination - which discussed in next section. This just covers the 

### LED Array Drivers 

* Side Button Array 
* Front Button Array
* Exterior Strips

### The Sensors Array

* Accelerometer - Virtual Tilt Plumb
* Time-of-Flight - Virtual Spring Ball Plunger

### The Power Array 

I'm using an upcycled Sony VGP-AC19V31 power adapter from an old laptop. It's 19.5V @ 4.7A (90W), delivered
via     barrel positive tip. I create two isolated power lines: 

* +12V High Amperage - for solenoids, motors, vibrators, shockers, & other haptics. May drive exterior LEDs. 
* +5V Mid-to-High Amperage - for many MCU units (like TLC5947). Also powers most of the button 5V LED arrays. 
* +3.3V Lo Amperage - Logic voltage, strip off w/ LDO!

<Discuss Circuit Here>

###

###

###

###

###

###

###



## Power

