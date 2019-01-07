#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#include <Adafruit_Sensor.h>
#include <DHT.h> // DHT for temperature and humidity.

#define DHT_PIN 12 // Pin which is connected to the DHT sensor.
#define OLED_RESET 4
#define DHTTYPE DHT22

#define I2C_ADDRESS_1 0x3d
#define I2C_ADDRESS_2 0x3c

#define SERIAL_BAUDRATE 9600

struct point {
  int x = 0;
  int y = 0;
};

struct eyelid {
  int y = 0;
  int angle = 0;
};

// Pin addresses (could be #defined instead)
const int SOIL_PIN = A0; // Analog input pin that the potentiometer is attached to
const int LED_PIN = LED_BUILTIN;

// Animation config
const long PAGE_INTERVAL = 10000; // Switch between stats and eyes every 10s
const long EYE_MOVE_MIN_INTERVAL = 500;
const long EYE_MOVE_MAX_INTERVAL = 4000;

const long EYE_BLINK_MIN_INTERVAL = 700;
const long EYE_BLINK_MAX_INTERVAL = 3000;

// Maximum number of pixels that can be jumped in one move
const int MAX_EYE_VELOCITY = 40;

// Animation working
unsigned long previousPageResetMillis = 0; // TODO Not previousPageChangeMillis?
unsigned long previousEyeMovementMillis = 0;
unsigned long previousEyeBlinkMillis = 0;

// TODO Currently both eyes/lids track the same coords, but it might be fun to give one for each eye
// TODO If things get really bad, for example, the eyes might want to point outwards
struct point currentPos; // Where are the eyes right now?
struct point futurePos; // Where are the eyes going to be next pass?
struct point targetPos; // Where are the eyes trying to get to? Updated with whenMoveEyes milliseconds has elapsed.

struct eyelid lid;

/* TODO Rename when better understood (if necessary) then reintroduce
int whenMoveEyes = 0;
int whenBlinkEyes = 0;
int view = 3;
int numViews = 3;
*/

// Sensor interfaces
// TODO This looks like an interface to read the dht sensor, is this C++?
DHT dht(DHT_PIN, DHTTYPE);

// Sensor readings
// TODO Understand the meaning of soilValue and soilMapped (TODO maybe rename to something a little more descriptive)
int soilValue = 0;        // value read from the pot
int soilMapped = 0;        // value read from the pot

// Display interfaces
// TODO What are these parameters: I2C addresses, clock pin, data pin (or data then clock)
// TODO Perhaps rename to displayLeft and displayRight
SSD1306Wire Display1(I2C_ADDRESS_1, D2, D1);
SSD1306Wire Display2(I2C_ADDRESS_2, D2, D1);

// Old variables, understand, maybe rename then get them into the above categories

const int soilInPin = A0; 

// TODO What is this?
uint32_t delayMS;

const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
// int ledState = LOW;             // ledState used to set the LED

unsigned long previousMillis = 0;        // last time page was reset
unsigned long previousEyesMillis = 0;        // will store last time page was reset
unsigned long previousBlinkMillis = 0;        // will store last time page was reset

// Number of millis for each view
const long pageInterval = 10000;
const long eyeMoveInterval = 4000;     
const long eyeBlinkInterval = 3000;          

int whenMoveEyes, whenBlinkEyes = 0;

// These variables are used to loop through different page types if I understand correctly
// For example, eyes = 0, data(temp + humidity) = 1, data(light + soil wetness) = 2, welcome page = 3
int view = 3;
int numViews = 3;


//Max eye speed
int v = 40;


// TODO Just didn't want to clash with setup but should be identical
void init() {
  // Prints out through the USB connection if there is one.
  // To use, call print(xyz);
  Serial.begin(SERIAL_BAUDRATE);

  // Have to initialise the temp + humidity sensor before we can start reading
  dht.begin();

  initDisplay(Display1);
  initDisplay(Display2);

  pinMode(ledPin, OUTPUT);
}

// TODO Check this is the correct way to declare a method/use parameters in C/C++
// TODO Particularly ensure we're passing by reference, rather than value
void initDisplay(SSD1306Wire display) {
  display.init();
  display.setI2cAutoInit(true); // TODO What is I2cAutoInit?
  display.flipScreenVertically();
}

void setup() {
  Serial.begin(9600);
  dht.begin();//Initialise the temerature and humidity Sensor.

  Display1.init();
  Display2.init();
  Display1.setI2cAutoInit(true);
  Display2.setI2cAutoInit(true);
  Display1.flipScreenVertically();
  Display2.flipScreenVertically();

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)

  // TODO What is going on here?
  pinMode(ledPin, OUTPUT);
}

void loop() {

  // Delay between measurements.
  // Wait a few milliseconds between measurements.
  // TODO Why?
  delay(20);

  // TODO Can we not be smarter with the above delay method to avoid passes that read the sensors but do not draw?
  // TODO For example, work out the next time something _would_ happen and delay until then?
  // TODO Or maybe, skip the sensor reads if we're on the eyes page?
  // TODO in C/C++ is there an equivalent of Java's final for local variables? It'd be nice to make this immutable.
  unsigned long currentMillis = millis();

  // TODO Might be a good idea to shuffle around sensor reads with display updates so we can implement a better waiting technique
  // TODO while ensuring sensor reading (slow) doesn't impact screen updates 

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity(); // TODO get a unit for this
  float temperatureCelsius = dht.readTemperature(); // Default unit is Celsius!

  soilValue = analogRead(SOIL_PIN);
  
  // Maps values from 650-710 range to human-readable 100-0
  soilMapped = map(soilValue, 650, 710, 100, 0);

  // Is it time to rotate the view?
  if (currentMillis - previousMillis >= PAGE_INTERVAL) {
    // Yes! Enough time has elapsed, move to the next page!
    view += 1;

    // We don't have a page 4 and we don't want to see page 3 again!
    if (view >= numViews) {
      view = 0;
    }
  }

  // Is it time to move the eyes?
  if (currentMillis - previousEyesMillis >= whenMoveEyes) {
    // TODO Get these magic numbers outta here
    futurePos.x = random(10,118);
    futurePos.y = random(10,54);
    whenMoveEyes = random(EYE_MOVE_MIN_INTERVAL, EYE_MOVE_MAX_INTERVAL);
    targetPos.x = futurePos.x;
    targetPos.y = futurePos.y;
    previousEyesMillis = currentMillis;
  }
 
  //update the blink time position
  // TODO Maybe a bug? Eye blinking looks like it's based on when the eyes last moved!
  if (currentMillis - previousEyesMillis >= whenBlinkEyes) {
    // TODO Get these magic numbers outta here
    whenBlinkEyes = random(EYE_BLINK_MIN_INTERVAL, EYE_BLINK_MAX_INTERVAL);
    lid.y = random(0,30);
    lid.angle = random(-10,10);

    //Begin Blink process and reset blink counter
    previousBlinkMillis = currentMillis;
  }

  switch (view) {
    case 0:
      currentPos = moveEyes(currentPos, targetPos);
      eyesView(currentPos, lid);  
      break;
    case 1:
      dataView(humValue, tempValue, soilMapped);
      break;
    case 2:
      emojiView(humValue, tempValue, soilMapped);
      break;      
    case 3:
      welcomeView();
      break;            
    }

}
void dataView(float humValue,float tempValue, float soilMapped){

    Display1.setColor(WHITE);
    Display1.clear();
    Display1.setFont(ArialMT_Plain_16);
    Display1.drawString(0, 32, "Humidity: " + String(humValue));
    Display1.display();
    
    Display2.setColor(WHITE);
    Display2.clear();
    Display2.setFont(ArialMT_Plain_16);
    Display2.drawString(0, 0, "Soil: " + String(soilValue));
    Display2.drawString(0, 32, "Temp: " + String(tempValue));
    Display2.display();

}

void emojiView(float humValue,float tempValue, float soilMapped){
  if (soilMapped >= 50){
    //happy
    
  }
  else{
    //sad and dry
    
  }
    Display1.setColor(WHITE);
    Display1.clear();
    Display1.setFont(ArialMT_Plain_16);
    Display1.drawString(0, 16, "Humidity: " + String(humValue));
    Display1.display();
    
    Display2.setColor(WHITE);
    Display2.clear();
    Display2.setFont(ArialMT_Plain_16);
    Display2.drawString(0, 0, "Soil: " + String(soilValue));
    Display2.drawString(0, 32, "Temp: " + String(tempValue));
    Display2.display();

}

void welcomeView(){

    Display1.setColor(WHITE);
    Display1.clear();
    Display1.setFont(ArialMT_Plain_24);
    Display1.drawString(0, 0, "YOPA");
    Display1.display();
    
    Display2.setColor(WHITE);
    Display2.clear();
    Display2.setFont(ArialMT_Plain_24);
    Display2.drawString(0, 0, "2019");
    Display2.display();


}

void eyesView(struct point currentPos, struct eyelid lid){
    Display1.clear();
    Display1.setColor(WHITE);
    Display1.fillRect(0, 0, 128, 64);
    Display1.setColor(BLACK);
    Display1.fillCircle(currentPos.x, currentPos.y,  20);
    Display1.setColor(WHITE);
    Display1.fillCircle(currentPos.x-10, currentPos.y-10, 10);
    Display1.setColor(BLACK);
    Display1.fillRect(0, 0, 128, lid.y);
    Display1.display();
    
    Display2.clear();
    Display2.setColor(WHITE);
    Display2.fillRect(0, 0, 128, 64);
    Display2.setColor(BLACK);
    Display2.fillCircle(currentPos.x, currentPos.y,  20);
    Display2.setColor(WHITE);
    Display2.fillCircle(currentPos.x-10, currentPos.y-10, 10);
    Display2.setColor(BLACK);
    Display2.fillRect(0, 0, 128, lid.y);
    Display2.display();

}


struct point moveEyes(struct point currentPos, struct point targetPos){
  //This function limits the maximum speed of the eyes to v pixels.
  int dx = targetPos.x-currentPos.x;
  int dy = targetPos.y-currentPos.y;
  float dr = sqrt(sq(dx)+ sq(dy));
  float ratio =  v/dr;
  //either mover the eyes to the new position or proportionally if the target position is too far away.
  if (ratio >= 1){
    currentPos = targetPos;
  }
  else{
    currentPos.x = currentPos.x+(ratio*dx);
    currentPos.y = currentPos.y+(ratio*dy);
  }
  return currentPos;
}
