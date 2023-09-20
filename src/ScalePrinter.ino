/*
 * Project ScalePrinter
 * Description:
 * Author:
 * Date:
 */

#include "adaqrcode.h"
#include "adalogo.h"
#include <SparkIntervalTimer.h>
#include <ParticleSoftSerial.h>
#include <Adafruit_Thermal.h>
#include <HX711ADC.h>
#include <ITEADLIB_Nextion.h>

bool debug = true;

#define PSS_RX D2
#define PSS_TX D3
#define mySerial softSerial

HX711ADC scale(A1, A0);
USARTSerial& nexSerial = Serial1;
SerialLogHandler logHandler;

uint8_t dtr = 255;

ParticleSoftSerial softSerial(PSS_RX, PSS_TX); 
Adafruit_Thermal printer;



double scaleValue = 0;
String buttonPressed = "No Button yet";
char radioSetBuffer[15] = {0};
String defaultRadio = "Boeuf Haché";


void b0PopCallback(void *ptr);
void b1PopCallback(void *ptr);

NexText weightText = NexText(1, 14, "weightText");
NexButton tareButton = NexButton(1, 2, "b0");
NexButton printButton = NexButton(1, 3, "b1");
NexVar radioText = NexVar(1,15,"va0");
NexPage scalePage = NexPage(1, 0, "MainMenu");
NexText titleText = NexText(1, 1, "t0");

NexTouch *nex_listen_list[] = 
{
    &tareButton,
    &printButton,
    NULL
};

void b0PopCallback(void *ptr)
{
    Log.info("Tare pressed");
    buttonPressed = "Tare Pressed";
    titleText.setText("Tare Pressed");
    scale.tare();

}
void b1PopCallback(void *ptr)
{
    Log.info("Print pressed");
    radioText.getText(radioSetBuffer,sizeof(radioSetBuffer));
    buttonPressed = "Print Pressed";
    titleText.setText("Print Pressed");
}

void testPrinterOnStartup(){
    // Test inverse on & off
printer.inverseOn();
printer.println(F("Inverse ON"));
printer.inverseOff();

// Test character double-height on & off
printer.doubleHeightOn();
printer.println(F("Double Height ON"));
printer.doubleHeightOff();

// Set text justification (right, center, left) -- accepts 'L', 'C', 'R'
printer.justify('R');
printer.println(F("Right justified"));
printer.justify('C');
printer.println(F("Center justified"));
printer.justify('L');
printer.println(F("Left justified"));

// Test more styles
printer.boldOn();
printer.println(F("Bold text"));
printer.boldOff();

printer.underlineOn();
printer.println(F("Underlined text"));
printer.underlineOff();

printer.setSize('L'); // Set type size, accepts 'S', 'M', 'L'
printer.println(F("Large"));
printer.setSize('M');
printer.println(F("Medium"));
printer.setSize('S');
printer.println(F("Small"));

printer.justify('C');
printer.println(F("normal\nline\nspacing"));
printer.setLineHeight(50);
printer.println(F("Taller\nline\nspacing"));
printer.setLineHeight(); // Reset to default
printer.justify('L');

// Barcode examples:
// CODE39 is the most common alphanumeric barcode:
printer.printBarcode("ADAFRUT", CODE39);
printer.setBarcodeHeight(100);
// Print UPC line on product barcodes:
printer.printBarcode("123456789123", UPC_A);

// Print the 75x75 pixel logo in adalogo.h:
printer.printBitmap(adalogo_width, adalogo_height, adalogo_data);

// Print the 135x135 pixel QR code in adaqrcode.h:
printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
printer.println(F("Adafruit!"));
printer.feed(2);

printer.sleep(); // Tell printer to sleep
delay(3000L); // Sleep for 3 seconds
printer.wake(); // MUST wake() before printing again, even if reset
printer.setDefault(); // Restore printer to defaults
}

void setup() {
    softSerial.begin(19200); // Initialize SoftwareSerial for Printer
    printer.begin(&softSerial);
    testPrinterOnStartup();
    
    if(debug == true){
        waitFor(Serial.isConnected, 5000);
    }
    defaultRadio.toCharArray(radioSetBuffer, 15);
    nexInit();
    Particle.syncTime();
    scalePage.show();
    Particle.variable("scale", scaleValue);
    Particle.variable("button", buttonPressed);
    Particle.variable("RadioSelected", radioSetBuffer);
    tareButton.attachPop(b0PopCallback);
    printButton.attachPop(b1PopCallback);
    weightText.setText("Nothing to weight");
    scale.begin();
    //scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.set_scale(475.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.tare();				        // reset the scale to 0
}

void loop() {
    nexLoop(nex_listen_list);
    scaleValue = scale.get_units(3);
    int scaleValueInt = scaleValue;
    String toShowString = String(scaleValueInt);
    toShowString = toShowString + "g";
    weightText.setText(toShowString);
    scale.power_down();			        // put the ADC in sleep mode
    delay(250);
    scale.power_up();
}