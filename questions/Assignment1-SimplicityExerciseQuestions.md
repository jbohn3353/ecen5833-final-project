Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**
   Answer: The system is consuming ~*5.03 mA* when no LEDs are on, and ~*5.55 mA* when a single LED is on with StrongStrong drive strength.


**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**
   Answer: The system is consuming ~*5.02 mA* when no LEDs are on, and ~*5.53 mA* when a single LED is on with WeakWeak drive strength. 


**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, referencing the main board schematic, WSTK-Main-BRD4001A-A01-schematic.pdf, and AEM Accuracy in the ug279-brd4104a-user-guide.pdf. Both of these PDF files are available in the ECEN 5823 Student Public Folder in Google drive at: https://drive.google.com/drive/folders/1ACI8sUKakgpOLzwsGZkns3CQtc7r35bB?usp=sharing . Extra credit is available for this question and depends on your answer.** 

   Answer: The AEM accuracy is listed .1 mA when measuring currents above 250 uA. Because of this, the measured values across the different drive strenghts are effectively the same in terms of what we can accurately measure with the AEM. *There is not a meaningful difference in current between the two*.

   The explanation for this is quite interesting. Drive strength, despite normally being listed as a current, is not some sort of fixed output or even a hard limit to prevent damage. Essentially, changing the drive strength changes the number of transistors being used to drive the GPIO pin, the more transistors, the lower the internal impedance, and the higher the pin's current output can be while the voltage remains within spec of the GPIO system. (The current value associated with the drive strength is the maximum current it can source without causing a voltage drop that would be out of spec).

   In this case, the stronger drive strength would imply a lower internal impedance, which should reduce the current consumption since the GPIO voltage should be fixed. However, the load resistance used to drive the LED is so much larger than this internal impedance that the impedance change that comes along with changing the drive strength is essentially insignificant, and that's why we don't see a difference.

   In order to confirm everything, we can take a look at some numbers. GPIO voltage should be the same as the board voltage which is listed as 3.3V in table 3.1 on page 10 of the user guide, this was measured as ~3.25V using a multimeter. Yellow LEDs tyically have a forward voltage drop of ~2V, but I measured that mine only dropped ~1.8V across them. Therefore, the load resistance has ~1.45V across it (which was also confirmed by measurement). Using Ohm's law with the measured voltage across the resistance and its value of 3k ohms from page 3 of the schematic, we can calculate that it's current consumption is ~.48 mA. Given the inaccuracy of the AEM measurement and the likely trace currents associated with the internals of the GPIO pin and the actual LED, this matches up almost perfectly with the difference in values reported by the AEM when the LED is off vs. on.


**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: With WeakWeak drive strength, the aveage current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% is ~*5.28 mA* (low avg of 5.02 mA, high avg of 5.53 mA)


**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: With WeakWeak drive strength, the aveage current for 1 complete on-off cycle for 2 LEDs with an on-off duty cycle of 50% is ~*5.52 mA* (low avg of 5.03 mA, high avg of 6.02 mA)


