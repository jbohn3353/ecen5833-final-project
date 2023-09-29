Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period?
   Answer: 28.69 uA
   <br>Screenshot:  
   ![Avg_current_per_period](screenshots/a4_period.png)  

2. What is the average current when the Si7021 is Powered Off?
   Answer: 8.18 uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](screenshots/a4_off.png)  

3. What is the average current when the Si7021 is Powered On?
   Answer: 555.49 uA
   <br>Screenshot:  
   ![Avg_current_LPM_On](screenshots/a4_on.png)  

4. How long is the Si7021 Powered On for 1 temperature reading?
   Answer: ~115 ms, the fact that this is longer that the 100 ms of our combined delays is likely due to the transaction time of the i2c transfers.
   <br>Screenshot:  
   ![duration_lpm_on](screenshots/a4_on.png)  

5. Compute what the total operating time of your design for assignment 4 would be in hours, assuming a 1000mAh battery power supply?
   Answer (in hours): 1000000 uAh / 28.69 uA = 34855.3502963
   
6. How has the power consumption performance of your design changed since the previous assignment?
   Answer: Old current consumption over the whole was 161.98 uA, so we improved by a factor of 161.98/28.69 = ~5.65
   


