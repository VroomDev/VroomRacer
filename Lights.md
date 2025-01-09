### Balancing LED Brightness for Perfect Color Mixing

At first when using a single RGB LED, all mt colors were mostly green. I realized that the green was too bright. So how to balance the colors fir the right blend?

When working with LEDs, you'll notice that the brightness can vary significantly based on the color of the light. This can pose a challenge when trying to mix LED lights to achieve a desired hue. For instance, in single bulb RGB LED setups, achieving proper color balance requires careful attention to the different brightness levels of each color.

To get the correct yellow color, for example, the green LED must have less current than the red LED to avoid having the color being mostly green. The chart below provides useful information for balancing the brightness of various LED colors:

| Color               | Lens Type                        | Forward Voltage (V) | Luminous Intensity (mcd) | Wavelength     | Avg mcd         | Balanced Ohm |
|---------------------|----------------------------------|---------------------|--------------------------|----------------|-----------------|--------------|
| White               | 5mm Round Top / Water Clear      | 3.0-3.2             | 12000-14000              | 6000-9000K     | 13000           | 788          |
| Warm White          |                                  | 3.0-3.2             | 14000-16000              | 3000-3500K     | 15000           | 909          |
| Red                 |                                  | 2.0-2.2             | 2000-3000                | 620-625nm      | 2500            | 152          |
| Yellow              |                                  | 2.0-2.2             | 1500-2000                | 585-595nm      | 1750            | 106          |
| Green               |                                  | 3.0-3.2             | 15000-18000              | 520-525nm      | 16500           | 1000         |
| Yellow Green        |                                  | 2.0-2.2             | 500-700                  | 570-575nm      | 600             | 36           |
| Blue                |                                  | 3.0-3.2             | 7000-8000                | 460-470nm      | 7500            | 455          |
| Orange              |                                  | 2.0-2.2             | 1500-2000                | 602-610nm      | 1750            | 106          |
| Ultraviolet (Purple)|                                  | 3.0-3.4             | 300-400                  | 395-400nm      | 350             | 21           |
| Pink                |                                  | 3.0-3.2             | 7000-8000                | —              | 7500            | 455          |

To achieve uniform brightness across different colors, you can either adjust the current programmatically or select appropriate resistors based on the chart above. This ensures that each LED color contributes correctly to the overall light hue, resulting in a balanced and visually pleasing output.

By paying attention to these details, you can create stunning LED displays with accurate and vibrant colors. Happy tinkering!


```
//// Adjust these values to balance the colors when using identical resistors
const uint8_t maxRed = 255;  // Full intensity for red
const uint8_t maxGreen = 39; //Reduced intensity for green to balance brightness
const uint8_t maxBlue = 85;// reduced intensity for blue

// Adjust these values to balance the colors when using near 145,1k,430 ohm resistors
const uint8_t maxRed = 244; //slightly reduced intensity for red
const uint8_t maxGreen = 255; // full intensity for green to balance brightness
const uint8_t maxBlue = 241;// reduced intensity for blue
