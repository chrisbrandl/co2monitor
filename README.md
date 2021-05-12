# co2monitor

An IoT project to counter the spread of Covid19 indoors. 

https://www.sciencedaily.com/releases/2021/04/210407143809.htm

There are lots of projects online that describe the basic setup to using a specific sensor for that, but they are mostly somewhat incomplete for practical use.

For this CO2 sensor project, a display is added to stay informed on the spot along with a colored light indicator to prominently indicate the severity of the current situation.

Furthermore, to have a record of the measurements and an easy means to analyse the incoming data, some MQTT functionality has been added to utimately rout the data e.g. into the ADAMOS IoT Platform, which enables me to automatically trigger actors based on the level of readings from this sensor.


# Hardware

The project 
- is run by an ESP8266 Mini D1
- leverages a MH-Z19 B/C sensor which measures the concentration of CO2 in ppm
- includes an OLED 128x64 display for showing the current readings
- uses a small 12xLED Ring to indicate the current conditions regarding the measured CO2 concentration
- Protoboard for integrating all wiring

# Wiring

Check the checked-in fritzing image for wiring illustration.

<table>
    <tr>
      <td>ESP</td><td>MHZ19</td> <td>OLED</td> <td>LEDRing</td>
    </tr>
    <tr>
      <td>5V</td> <td>VIN</td>---<td>VDD</td>---<td>VIN</td>
    </tr>
    <tr>
      <td>G</td> <td>GND</td>---<td>GND</td>---<td>GND</td>
    </tr>
    <tr>
      <td>D1</td> <td>---</td> <td>SCK</td> <td>---</td> 
    </tr>
    <tr>
      <td>D2</td> <td>---</td> <td>SDA</td> <td>---</td> 
    </tr>
    <tr>
      <td>D3</td> <td>TX</td> <td>---</td><td>---</td> 
    </tr>
    <tr>
      <td>D4</td> <td>RX</td> <td>---</td><td>---</td> 
    </tr>
    <tr>
      <td>D5</td> <td>---</td><td>---</td> <td>IN</td>
    </tr>
</table>

# Code

See the code in the master branch, need to add WLAN access point credentials and MQTT details.


# 3D Model

This project is re-using this great modular 3D model for IoT devices:

https://www.thingiverse.com/thing:3391397

The dome part has been modified to ensure free circulation of air around the sensor, see checked-in STL file. 
The model is printed e.g. on a standard Ender 3 printer.

# Dashboarding 

Once the data is being sent to an MQTT Server, it can be used via Node Red to be forwarded to the e.g. ADAMOS IIoT Platform. 

These are some basic steps on Node Red:

* Subscribe to MQTT Topic  
* Build JSON payload
* POST to ADAMOS IIoT Tenant as measurement
* Add widget for incoming data points on IoT platform

Resulting UI screenshot is available in the Repository.

