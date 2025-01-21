//////// Lights by Chris Busch
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */


//| Color               | Lens Type                        | Forward Voltage (V) | Luminous Intensity (mcd) | Wavelength     |
//|---------------------|----------------------------------|---------------------|--------------------------|----------------|
//| White               | 5mm Round Top / Water Clear      | 3.0-3.2             | 12000-14000              | 6000-9000K     |
//| Warm White          |                                  | 3.0-3.2             | 14000-16000              | 3000-3500K     |
//| Red                 |                                  | 2.0-2.2             | 2000-3000                | 620-625nm      |
//| Yellow              |                                  | 2.0-2.2             | 1500-2000                | 585-595nm      |
//| Green               |                                  | 3.0-3.2             | 15000-18000              | 520-525nm      |
//| Yellow Green        |                                  | 2.0-2.2             | 500-700                  | 570-575nm      |
//| Blue                |                                  | 3.0-3.2             | 7000-8000                | 460-470nm      |
//| Orange              |                                  | 2.0-2.2             | 1500-2000                | 602-610nm      |
//| Ultraviolet (Purple)|                                  | 3.0-3.4             | 300-400                  | 395-400nm      |
//| Pink                |                                  | 3.0-3.2             | 7000-8000                | —              |

//| Color               | Lens Type                        | Forward Voltage (V) | Luminous Intensity (mcd) | Wavelength     | Avg mcd         |balanced ohm
//|---------------------|----------------------------------|---------------------|--------------------------|----------------|-----------------|------------
//| White               | 5mm Round Top / Water Clear      | 3.0-3.2             | 12000-14000              | 6000-9000K     | 13000           |788
//| Warm White          |                                  | 3.0-3.2             | 14000-16000              | 3000-3500K     | 15000           |909
//| Red                 |                                  | 2.0-2.2             | 2000-3000                | 620-625nm      | 2500            |152
//| Yellow              |                                  | 2.0-2.2             | 1500-2000                | 585-595nm      | 1750            |106
//| Green               |                                  | 3.0-3.2             | 15000-18000              | 520-525nm      | 16500           |1000
//| Yellow Green        |                                  | 2.0-2.2             | 500-700                  | 570-575nm      | 600             |36
//| Blue                |                                  | 3.0-3.2             | 7000-8000                | 460-470nm      | 7500            |455
//| Orange              |                                  | 2.0-2.2             | 1500-2000                | 602-610nm      | 1750            |106
//| Ultraviolet (Purple)|                                  | 3.0-3.4             | 300-400                  | 395-400nm      | 350             |21
//| Pink                |                                  | 3.0-3.2             | 7000-8000                | —             | 7500            |455



// Define an enum to represent different colors
typedef enum  {
  RED,
  GREEN,
  BLUE,
  YELLOW,
  ORANGE,
  PURPLE,
  CYAN,
  WHITE,
  BLACK,
  NUM_COLORS // Keeps track of the number of colors
} Color;


// Define a struct to hold RGB values
struct RGB {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};



// Array of short 3-letter labels for each color 
const char* colorLabels[NUM_COLORS] = { "RED", "GRN", "BLU", "YLW", "ORG", "PUR", "CYN", "WHT", "BLK" };

//// Adjust these values to balance the colors when using identical resistors
//const uint8_t maxRed = 255; //244;  //230 Full intensity for red
//const uint8_t maxGreen = 39; //64; //60// Reduced intensity for green to balance brightness
//const uint8_t maxBlue = 85;//255; //240;  // Slightly reduced intensity for blue

//// Adjust these values to balance the colors when using near 152,1k,454 ohm resistors
//const uint8_t maxRed = 255; //244;  //230 Full intensity for red
//const uint8_t maxGreen = 255; //64; //60// Reduced intensity for green to balance brightness
//const uint8_t maxBlue = 255;//255; //240;  // Slightly reduced intensity for blue

//// Adjust these values to balance the colors when using near 145,1k,430 ohm resistors
const uint8_t maxRed = 244; //  intensity for red
const uint8_t maxGreen = 255; //intensity for green
const uint8_t maxBlue = 241; //intensity for blue


// Create an array of RGB structs to hold custom color values
const RGB colorTable[NUM_COLORS] = {
  {maxRed, 0, 0},             // RED
  {0, maxGreen, 0},           // GREEN
  {0, 0, maxBlue},            // BLUE
  {maxRed, maxGreen*(int)3/4, 0},      // YELLOW
  {maxRed, maxGreen * (int)1 / 5, 0}, // ORANGE
  {maxRed / 2, 0, maxBlue / 2}, // PURPLE
  {0, maxGreen, maxBlue},     // CYAN
  {maxRed, maxGreen, maxBlue},// WHITE
  {0, 0, 0}                   // BLACK
};

// Static brightness value ranging from 0 to 8
#define MAXBRIGHTNESS 8

class Lights {
  public:

  byte redPin,bluePin,greenPin;



  void setup(byte r,byte g,byte b){
    redPin=r;    
    greenPin=g;
    bluePin=b;
    // Initialize the RGB LED pins as outputs
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
  }


  



  void demo() {
    for(int c=0;c<NUM_COLORS;c++){
      if(serialOn) Serial.println(colorLabels[c]);
      setColor(colorTable[c]);
      delay(1000);
    }
    
  }

  void setColor(RGB rgb){
    analogWrite(redPin, rgb.red * brightness / MAXBRIGHTNESS);
    analogWrite(greenPin, rgb.green * brightness / MAXBRIGHTNESS);
    analogWrite(bluePin, rgb.blue * brightness / MAXBRIGHTNESS);
  }
  
  // Function to set the color of the RGB LED with brightness adjustment
  void setColor(Color color) {
    RGB rgb = colorTable[color];
    setColor(rgb);  
  }


  
};
