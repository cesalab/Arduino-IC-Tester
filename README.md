![ArduinoICT](img/ArduinoICT.jpg)

Quite some time ago I purchased an IC Tester from Genius. The G540 is able to programm various IC as it is able to test CMOS and TTL IC. Last option was quite interesting to me as this makes repairing stuff a lot easier when you know which part is defect instead of exchanging all part just by trial and error.

The programmer did a quite good job (some IC were not recognized) until I had to upgrade to Win7.

Here the programmer started to create troubles, the programmer was not recognized in some cases and the program froze during IC testing. After searching for alternatives I decided to make my own tester with some additional advantages.

So the result was a Arduino based IC-tester with an optional Serial output which does the job in most cases (still some room for improvement available).

![G540](img/G540.jpg)

The original tester did a quite good job exept, that you had a lot of clicks (selecting the device etc.) before you could start, you allway needed to run the programm itself and most important:

there was no information about the testing result. If a IC was not found it was not possible to identify if it was not found due to being defect or due to a incorrect testing cycle (which appears to happen for some ICs).

So the idea was to overcome these disadvantages by develloping a tester of my own based on a Arduino Nano. 

![Diagram IC-Tester](img/Diagram.jpg)

The circuit is quite easy.

First of all there is the Arduino Nano. Due to the amount of available ports the maximum of pins to be tested is 16 (which is enough for most IC). To achieve this, the communication to the LCD display and to the EEPROM containing the test-data is done via I2C. The Nano takes over the communication to the computer to show the detailed test results.

The LCD-display is a simple standard 16x2 display including a I2C converter, thus needing only two pins of the arduino.

The test data is stored in a serial I2C EEPROM AT24C512. Here a script is stored which is tested step by step. For every type of IC a sequence of logical inputs to set and outputs to be expected. In case the outputs do not match the expectations, the script will jump to next possible part. In the current version the EEPROM needs to be programmed seperately via a programmer. I did not find a solution of transfering 25kbytes of data via the serial terminal.

The test script is in clear text so can be modified quite easily. The syntax is in the arduino sketch.

While testing multiple signals are set to the tested part which do not match the specification of the part (e.g. low is set as an input to a pin which acts as a high output) because all possible combinations are tested. To prevent overloading the Arduino and the part, all connections are done by 680 Ohms resistors. This creates a lot of "below the specifications" signals thus leading to random outputs of the tested IC. Still, if the IC maches to tested signals, the output of the tested IC is usable.

The test ist started with a single switch connected to one of the signle use analogue inputs.

___

In the linked video the tester can be seen in operation.

Similar to the original the tester does not find all required IC. Some are a bit difficult in regard to what signals shoulb be expected. I will do some optimization as I find some spare time.

