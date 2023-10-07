Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.*

1. Provide screen shot verifying the Advertising period matches the values required for the assignment.
   Answer: 260 ms
   <br>Screenshot:  
   ![advertising_period](screenshots/a5_ad_period.png)

2. What is the average current between advertisements (Don't measure an interval with a LETIMER UF event)?
   Answer: 5.24 uA
   <br>Screenshot:  
   ![avg_current_between_advertisements](screenshots/a5_ad_off.png)  

3. What is the peak current of an advertisement? 
   Answer: 25 mA
   <br>Screenshot:  
   ![peak_current_of_advertisement](screenshots/a5_ad_on.png)  

4. Provide screen shot showing the connection interval settings. Do they match the values you set in your slave(server) code or the master's(client) values?.
   <br>Screenshot: 
   ![connection_interval](screenshots/a5_settings.png)  

5. What is the average current between connection intervals (Don't measure an interval with a LETIMER UF event)?
   Answer: 4.93 uA
   <br>Screenshot:  
   ![avg_current_between_connection_intervals](screenshots/a5_conn_off.png)  

6. If possible, provide screen shot verifying the slave latency matches what was reported when you logged the values from event = gecko_evt_le_connection_parameters_id. 
   Answer: 30 ms = slave latency = conn interval
   <br>Screenshot: 
   ![slave_latency](screenshots/a5_conn_period.png)  

Just for fun, here's a screenshot of the energy profiler with an indication being sent:
<br>
![indication](screenshots/a5_indication.png)

