/***************************************************************************
 *
 * @author Rachelle Burgos (rachelle.burgos27@myhunter.cuny.edu)
 * @date   12/21/2022
 * @brief  Arduino-based project that uses a MPU-6050 to detect poor posture
 *         and a haptic buzzer to alert the user to correct their posture.
 *         Features a 1.69" IPS Display to display a message insulting 
 *         (or complementing!) the wearer depending on whether or not they 
 *         are slouching.
 *
 *         Created for CSCI-267 Embedded Systems at Hunter College (Fall 2022)
 *         Instructor: Eric Schweitzer
 *
 *
 *         External libraries used:
 *         - MPU6050_light by rfetick on GitHub      ( https://github.com/rfetick/MPU6050_light/ )            , downloaded on 12/18/2022
 *         - Adafruit_DRV2605 by AdaFruit Industries ( https://github.com/adafruit/Adafruit_DRV2605_Library ) , downloaded on 11/16/2022
 *         - Adafruit_GFX by AdaFruit Industries     ( https://github.com/adafruit/Adafruit-GFX-Library )     , downloaded on 11/28/2022
 *         - Adafruit_ST7789 by AdaFruit Industries  ( https://github.com/adafruit/Adafruit-ST7735-Library )  , downloaded on 11/28/2022
 *
 *
 *         Some code adapted from:
 *         - GetAngle example sketch from MPU6050_light library
 *         - graphicstest example sketch from Adafruit_ST7789 library
 *
 ***************************************************************************/


#include <Wire.h>             // I2C default library from Arduino
#include <MPU6050_light.h>    // Hardware-specific library for MPU6050
#include <Adafruit_DRV2605.h> // Hardware-specific library for DRV2605
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <SPI.h>

// Macros for TFT Display initialization
#define TFT_CS        10
#define TFT_RST        9
#define TFT_DC         8

// Colour definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF


// Create objects for every module
MPU6050 mpu(Wire);                                              // MPU6050 6-DoF Gyroscope-Accelerometer
Adafruit_DRV2605 drv;                                           // DRV2605 Haptic Controller
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); // ST7789 TFT Display


float x_angle   = 0.0, // Pitch
      y_angle   = 0.0; // Roll

const unsigned int slouch_angle = 3; // Threshold 


/**
 * @post Change the TFT text settings to the values provided in the parameters.
 * @param colour the new colour of the text
 * @param size   (optional) the new size of the text  
 */
void changeTextSettings(uint16_t colour, uint8_t size = 1) {
  tft.setTextColor(colour);
  tft.setTextSize(size);
}

/**
 * @return Return true if the wearer is slouching, false otherwise. 
 */
bool isSlouching() {
  if( x_angle >= slouch_angle ) 
    { return true; } 
  else 
    { return false; }
}

/**
 * @post MPU fetches the new values from the sensor and x_angle, y_angle variables are updated with the new angle values.
 */
void updateMPU() {
  mpu.update(); // Fetch the new values from the sensor
  x_angle = mpu.getAngleX(); // Store the updated angle values
  y_angle = mpu.getAngleY();
}

/**
 * @post  go() method is called on the DRV2605, causing it to playback the waveforms.
 *        TFT LCD display changes text to scold the wearer. 
 */
void alert() {
  tft.fillScreen(BLACK); // Clear the screen
  changeTextSettings(RED, 3);

  tft.println(F("Look at me, I'm an idiot with terrible posture. Point and laugh!!!"));

  while(isSlouching()) {
    updateMPU();
    drv.go();
    // Debug prints to the serial monitor
    Serial.print(F("Poor posture... "));
    debugPrint_X_Y();
  }
  tft.fillScreen(BLACK);
  changeTextSettings(WHITE, 2);
} // End alert()

/**
 * @post Prints the X and Y angles to the serial monitor
 */
void debugPrint_X_Y() {
  Serial.print(F("x angle:\t"));
  Serial.print(x_angle);
  Serial.print(F("\ty angle:\t"));
  Serial.println(y_angle);
}

// ------- setup() -------
void setup() {
  Serial.begin(115200); // Start serial monitor @ Baud rate 9600
  Wire.begin(); // Begin I2C communication
  
  // ------- Set up TFT Display -------
  // Connect to TFT Display (with the correct dimensions)
  tft.init(240, 280); 

  tft.setTextWrap(true); // Enable text wrap
  tft.fillScreen(BLACK); // Clear the screen
  tft.setCursor(0, 30);  // Set the cursor to x: 0 y: 30 on the TFT display

  // Print text to the TFT display
  changeTextSettings(CYAN, 3); 
  tft.println("Hello World!");

  changeTextSettings(WHITE, 2);
  tft.println("Setting up...\n\n");

  tft.setTextSize(1);

  // ------- Set up MPU 6050 -------
  // Connect to MPU6050
  byte status = mpu.begin();
  if (status != 0) {
    Serial.println(F("Failed to find MPU6050")); // Failed to find the device
    tft.println(F("Failed to find MPU6050"));
    while (1) delay(10);
  }
  Serial.println(F("MPU6050 Found!"));
  tft.println(F("MPU6050 Found!"));
  // Calculate MPU6050 offsets
  Serial.println(F("Calculating offsets... Please stay still..."));
  tft.println(F("Calculating offsets...  Please stay still..."));
  mpu.calcOffsets(); // Calculate gyroscope and accelerometer offsets
  delay(2000);
  Serial.println(F("Done! Offsets calculated.\n"));
  tft.println(F("Done! Offsets calculated.\n"));


  // ------- Set up DRV2605 Haptic Controller -------
  // Connect to DRV2605
  if (!drv.begin()) {
    Serial.println(F("Could not find DRV2605")); // Failed to find the device
    tft.println(F("Could not find DRV2605"));
    while (1) delay(10);
  }
  Serial.println(F("DRV2605 Found!"));
  tft.println(F("DRV2605 Found!"));

  drv.selectLibrary(1); // Select the TS2200 A waveform library 
    /** Other library selection values can be found at 
     * Section 7.6.4 https://cdn-shop.adafruit.com/datasheets/DRV2605.pdf
     */
  
  drv.setWaveform(0, 14); // Select the 'Strong Buzz - 100%' waveform
    /** Other effects from the waveform library effects can be found at 
     * Section 11.2 https://cdn-shop.adafruit.com/datasheets/DRV2605.pdf
     */

  drv.setMode(DRV2605_MODE_INTTRIG); // Set the device mode to internal trigger; call 'go()' to start playback  

  changeTextSettings(GREEN, 2);
  tft.println(F("\nDevice ready!"));
  delay(2000); // Leave the message on the screen for 2s

  tft.fillScreen(BLACK); // Clear the screen
  delay(2000); // Delay for 2s prior moving on to loop().
} // End setup()


// ------- loop() -------
void loop() {
  tft.setCursor(0, 30); // Set the cursor to x: 0 y: 30 on the TFT display 
                        // Must be done in loop() to prevent the text from going out-of-bounds
  
  updateMPU(); // Update the MPU and x, y angle values

  if ( isSlouching() ) { // Alert the wearer if they are slouching 
    alert(); 
  } else { // Otherwise, stop the motor and change the display's text
    drv.stop();
    tft.println(F("Look at me and my impeccable posture. You all are so jealous of me."));
    // Debug prints to the serial monitor
    Serial.print(F("Good posture... "));
    debugPrint_X_Y();
  } 
} // End loop()