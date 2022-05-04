Add the following code to FX.cpp overiding the Aurora effect:



const boolean segmentArray[11][7] = {{0, 0, 0, 0, 0, 0, 0}, {1, 1, 1, 0, 1, 1, 1}, {1, 0, 0, 0, 1, 0, 0},
                                     {1, 1, 0, 1, 0, 1, 1}, {1, 1, 0, 1, 1, 1, 0}, {1, 0, 1, 1, 1, 0, 0},
                                     {0, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 1, 1, 1, 1}, {1, 1, 0, 0, 1, 0, 0},
                                     {1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 0}
};
#define USE_COLON_SEPARATOR

uint16_t WS2812FX::mode_aurora(void) {
     uint8_t timeSecond = second(localTime);
  uint8_t timeMinute = minute(localTime);

  uint8_t ledsPerSegment = 9;       //9 LEDs per Segmentfraction (7 Segment with each 9 LEDs = 63 LEDs)
  uint8_t numberOfSegments = 4;     //2x Hours, 2x Minutes

  uint8_t timeHour = 0;             //Bypass compiler error
  uint8_t limit = 0;                //Bypass compiler error, sets it according to 12/24h format
  
  if(!useAMPM) {
    limit = 0;
    timeHour = hour(localTime);
  }
  else {
    limit = 1;
    timeHour = hour(localTime) % 12;
    if(timeHour == 0) timeHour = 12;
  }

  #ifdef USE_COLON_SEPARATOR
    uint8_t ledsPerDot = 1;         //1 LED per seconds dot, so 2 in total
    #define dotColor 0x999999
  #endif


  uint8_t digits[4];
  digits[0] = timeHour / 10;
  digits[1] = timeHour % 10;
  digits[2] = timeMinute / 10;
  digits[3] = timeMinute % 10;

  /*digits[3] = timeHour / 10;
  digits[2] = timeHour % 10;
  digits[1] = timeMinute / 10;
  digits[0] = timeMinute % 10;*/
  //Reversed order for my own setup

  uint8_t index = 0;
  uint8_t usedIndex = 0;        //Used for better Color Palette usage
  uint8_t counter = (now * ((SEGMENT.speed >> 2) + 2)) & 0xFFFF;
  
  for (int i = numberOfSegments - 1; i >= limit; i--) {      //Cycle through digits
    for (uint8_t j = 0; j < 7; j++) {                   //Cycle through each part of a digit
      for (uint8_t k = 0; k < ledsPerSegment; k++) {    //Cycle through each led per part of a digit
        if(segmentArray[digits[i]+1][j]) {          //If LEDs should light up:
          usedIndex++;
          if(!useAMPM) {
            if(i == 3 || i == 2) {      //Minutes
              setPixelColor(index, color_from_palette(usedIndex*map(SEGMENT.intensity, 0, 255, 0, 25)+(now / map(SEGMENT.speed, 0, 255, 400, 5)), false, true, 0));
            }
            else if(i == 1 || i == 0) { //Hours
              setPixelColor(index, color_from_palette(usedIndex*map(SEGMENT.intensity, 0, 255, 0, 25)+(now / map(SEGMENT.speed, 0, 255, 400, 5)), false, true, 1));
            }
          }
          else {
            if(i == 3 || i == 2) {      //Minutes
              setPixelColor(index, color_from_palette(usedIndex*map(SEGMENT.intensity, 0, 255, 0, 25)+(now / map(SEGMENT.speed, 0, 255, 400, 5)), false, true, 0));
            }
            else if(i == 1) {           //Hours
              setPixelColor(index, color_from_palette(usedIndex*map(SEGMENT.intensity, 0, 255, 0, 25)+(now / map(SEGMENT.speed, 0, 255, 400, 5)), false, true, 1));
            }
          }
        }
        else {      //LEDs go black
          setPixelColor(index, 0);
        }
        index++;
      }
    }
    #ifdef USE_COLON_SEPARATOR
      if(numberOfSegments - 2 == i) {
        if(timeSecond % 2 == 0) {                                                           //Light up on even seconds
          for(uint8_t l = 0; l < ledsPerDot*2;l++) {
            setPixelColor(index, dotColor);
            index++;
          }
        }
        else {
          for(uint8_t l = 0; l < ledsPerDot*2;l++) {
            setPixelColor(index, 0);
            index++;
          }
        }
      }
    #endif
  }
  if(useAMPM) {
    if(digits[0] > 0) {
      for(uint8_t i = 0;i < 2*ledsPerSegment;i++) {
        setPixelColor(index+i, color_from_palette(usedIndex*map(SEGMENT.intensity, 0, 255, 0, 25)+(now / map(SEGMENT.speed, 0, 255, 400, 5)), false, true, 1));
      }
    }
    else {
      for(uint8_t i = 0;i < 2*ledsPerSegment;i++) {
        setPixelColor(index+i, 0);
      }
    }
    index += 2*ledsPerSegment;
  }

  
  for(uint16_t i = index; i < SEGLEN;i++) {
    setPixelColor(i, SEGCOLOR(2));
  }

  return FRAMETIME;
}