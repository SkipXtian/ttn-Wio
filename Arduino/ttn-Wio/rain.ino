//****************************RAIN*****************************


#define RAIN_FACTOR 28 //0.2794  is actual

volatile uint32_t rainTipper=0;
volatile uint32_t rainLastTip=0;

uint32_t getRainCount()
{   
  uint32_t rainTipperCount = rainTipper;
  rainTipper=0;
  return rainTipperCount;
}

uint32_t calcUnitRain()
{   
  uint32_t unitRain = rainTipper * RAIN_FACTOR;
  
  rainTipper=0; 
  
  DEBUG_PRINT(F("Rain Fall :"));
  DEBUG_PRINT(unitRain);
  DEBUG_PRINTLN(F("mm"));
  return unitRain;
}

void rainGaugeISR()
{
  if(millis()-rainLastTip>400) {  //Stops debounce of the reed switch
    rainTipper += 1;
    rainLastTip=millis();
  }
}
