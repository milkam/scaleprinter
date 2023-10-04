/*
 * Project ScalePrinter
 * Description:
 * Author: Michel Tremblay
 * Date:
 */

#include <SparkIntervalTimer.h>
#include <ParticleSoftSerial.h>
#include <Adafruit_Thermal.h>
#include <HX711ADC.h>
#include <ITEADLIB_Nextion.h>
#include "Particle.h"
#include <SparkJson.h>
#include "ThermalPrinterTest.h"

bool debug = true;

#define PSS_RX D2
#define PSS_TX D3
#define mySerial softSerial

HX711ADC scale(A1, A0);
USARTSerial &nexSerial = Serial1;
SerialLogHandler logHandler;

uint8_t dtr = 255;

ParticleSoftSerial softSerial(PSS_RX, PSS_TX);
Adafruit_Thermal printer;
String baseButtonsData = "{\"buttons\" : [{\"name\" : \"button1\",\"Boeuf Haché\" : \"button1Value\"},{\"name\" : \"button2\",\"value\" : \"Porc Haché\"},{\"name\" : \"button3\",\"value\" : \"Viande Baozi\"}]}";
const char *PUBLISH_EVENT_NAME = "scalePrinterData";
double scaleValue = 0;
String buttonPressed = "No Button yet";
char radioSetBuffer[15] = {0};
String defaultRadio = "Boeuf Haché";
String buttonsJson = String(500);
String buttonsJson2 = String(500);

void b0PopCallback(void *ptr);
void b1PopCallback(void *ptr);

NexText weightText = NexText(1, 14, "weightText");
NexButton tareButton = NexButton(1, 2, "b0");
NexButton printButton = NexButton(1, 3, "b1");
NexVar radioText = NexVar(1, 15, "va0");
NexPage scalePage = NexPage(1, 0, "MainMenu");
NexText titleText = NexText(1, 1, "t0");

NexTouch *nex_listen_list[] =
    {
        &tareButton,
        &printButton,
        NULL};

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
    radioText.getText(radioSetBuffer, sizeof(radioSetBuffer));
    buttonPressed = "Print Pressed";
    titleText.setText("Print Pressed");
}

void myHandler(const char *event, const char *data)
{
    StaticJsonBuffer<500> jsonBuffer;
    char *mutableCopy = strdup(data);
    JsonObject &root = jsonBuffer.parseObject(mutableCopy);
    free(mutableCopy);
    if (atoi(root["buttons"]) == NULL)
    {
        publishData();
        delay(1000);
        Particle.publish("getButtons", buttonsJson, PRIVATE);
    }
    else
    {
        buttonsJson2 = atoi(root["buttons"]);
    }
}
void publishData()
{
    // Create the Base buttons
    char buf[500];
    snprintf(buf, sizeof(buf), baseButtonsData);
    Serial.printlnf("publishing %s", buf);
    Particle.publish(PUBLISH_EVENT_NAME, buf, PRIVATE);
}

void setup()
{

    Particle.subscribe("hook-response/getButtons", myHandler, MY_DEVICES);
    Particle.publish("getButtons", buttonsJson, PRIVATE);
    softSerial.begin(19200); // Initialize SoftwareSerial for Printer
    printer.begin(&softSerial);
    testPrinterOnStartup(printer);

    if (debug == true)
    {
        waitFor(Serial.isConnected, 5000);
    }

    defaultRadio.toCharArray(radioSetBuffer, 15);
    nexInit();
    Particle.syncTime();
    scalePage.show();
    Particle.variable("scale", scaleValue);
    Particle.variable("button", buttonPressed);
    Particle.variable("RadioSelected", radioSetBuffer);
    Particle.variable("buttons", buttonsJson);
    Particle.variable("buttons2", buttonsJson2);
    tareButton.attachPop(b0PopCallback);
    printButton.attachPop(b1PopCallback);
    weightText.setText("Nothing to weight");
    scale.begin();
    // scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.set_scale(475.f); // this value is obtained by calibrating the scale with known weights; see the README for details
    scale.tare();           // reset the scale to 0
}

void loop()
{
    nexLoop(nex_listen_list);
    scaleValue = scale.get_units(3);
    int scaleValueInt = scaleValue;
    String toShowString = String(scaleValueInt);
    toShowString = toShowString + "g";
    weightText.setText(toShowString);
    scale.power_down(); // put the ADC in sleep mode
    delay(250);
    scale.power_up();
}
