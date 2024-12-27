# VroomRacer
Vroom Racer is a simple Arduino Lap Timer for slot car racing using photoresistor photo eyes.

# Arduino-Based Slot Car Tracker and Timer

## Project Goals
The aim of this project is to create an Arduino-based slot car tracker and timer that provides reliable counting and timing in a compact form. Using a laptop is impractical due to space constraints and the inconvenience of using Windows. By coding my own solution, I can tailor it to my specific needs.

## Race Control Lights
Each driver has three lights: red, yellow, and green.
- **Red**: Your car should be stopped. Laps are not counted.
- **Yellow**: Your car should go at half speed through the lap sensor.
- **Green**: The race is on!

## Lap Timing and Speed Calculation
- **Lap Duration**: Measured from start to start time in milliseconds.
- **Trap Speed**: Calculated based on the time the car breaks the beam. The car length is assumed to be 2.5 inches, and this is internally calculated as a count of interrupt "clicks".

## Real-Time Calculations
- **Clipped Averages**:
  - For 3 or more laps: $$\((\text{accumulatedLapDuration} - \text{worstLapDuration} - \text{bestLapDuration}) / (\text{completedLaps} - 2)\)$$
  - For 2 laps: Simple average

- **False Start**: If the first lap is out of range (either too fast or too slow), the race is red-flagged, declared a false start, and then restarted.
- **Impossibly Fast Laps**: Any lap that is impossibly fast is deleted.

## Automatic Yellow Flags
- If a driver hasn't completed a lap after 1.5 times the clipped average lap time for all racers, a yellow light goes on.
- If another driver's next lap time is too fast during a yellow, their lap could be deleted as a penalty. A lap is too fast if it's less than $$\(\text{avgdur} - (\text{lapFinishClock} - \text{yellowStartClock}) \times 2\)$$.

## Track and Steward
- **Pseudo "Mobius" Track**: A 4-lane track that is actually a 2-lane track.
- **A.I. Steward**: Reviews any new shortest lap after lap 1. If the lap is less than half the average duration, it will be deleted as it indicates a jumped lane or rider.

## Race Management
- **Crash on Lap 1**: Push reset to restart the race.
- **Pit Stop Lap**: If a call to pit lap happens, a pitted lap takes 10 seconds. A call to pit is indicated by a red light for the driver. A lap won't be counted as long as the red light is on. Once it goes out, the lap will be counted as a pit stop. The strategy is to stop just before the counter and wait.

## Light Signals
- **Both Red Lights**: Race stopped/restart if all laps haven't been completed.
- **Both Solid Yellow Lights**: Go slow.
- **Both Green Lights**: Go!

- **Single Red Light**: Pit lap, wait for it to go green before crossing the finish line.
- **Single Green Blinking**: You won.
- **Single Red Blinking**: You lost.
- **Single Yellow Blinking**: Lap ignored (deleted).

## Sounds
- **Single Tone**: Lap counted.
- **Start**: 3 starting beeps.
- **Winner**: Ode to Joy.
- **Lost**: TBD.
- **Pit Stop Song**: Engine sound.
- **Lap Deletion Song**: TBD.
