unsigned long lastUpdate = 0;
const unsigned long threshold = 2000;
int ledIndex = 0;


void renderOne(){
  if (currentColorCount < 1) {
    return;
  }

  unsigned long timeNow = millis();

  if (timeNow - lastUpdate < threshold ){
    return;
  }

  lastUpdate = timeNow;

  int randIndex = random(currentColorCount);
    
    strip.setPixelColor(
      ledIndex,
      strip.Color(
        currentColors[randIndex].r,
        currentColors[randIndex].g,
        currentColors[randIndex].b
        )
      );

    strip.show();
    ledIndex++;

  if (ledIndex >= NUM_LEDS){
    ledIndex = 0;
  }
}