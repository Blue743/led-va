void renderColorCycle() {
  if (currentColorCount < 2) {
    return;
  }

  if (!cycleInitialized) {
    cycleOrigin = random(currentColorCount);

    do {
      cycleDestination = random(currentColorCount);
    } while (cycleDestination == cycleOrigin);

    cyclePrevious = -1;
    cycleProgress = 0.0f;
    cycleInitialized = true;
  }

  unsigned long now = millis();

  if (now - lastCycleUpdateMs < fadeSpeed) {
    return;
  }

  lastCycleUpdateMs = now;

  int newR =
    currentColors[cycleOrigin].r +
    (currentColors[cycleDestination].r -
     currentColors[cycleOrigin].r) *
    cycleProgress;

  int newG =
    currentColors[cycleOrigin].g +
    (currentColors[cycleDestination].g -
     currentColors[cycleOrigin].g) *
    cycleProgress;

  int newB =
    currentColors[cycleOrigin].b +
    (currentColors[cycleDestination].b -
     currentColors[cycleOrigin].b) *
    cycleProgress;

  for (int i = 0; i < NUM_LEDS; i++){
    strip.setPixelColor(
      i,
      strip.Color(newR, newG, newB)
    );
  }

  strip.show();

  cycleProgress += cycleProgressStep;

  if (cycleProgress >= 1.0f) {
    cyclePrevious = cycleOrigin;
    cycleOrigin = cycleDestination;

    int candidate;

    do {
      candidate = random(currentColorCount);
    } while (
      candidate == cycleOrigin ||(
        currentColorCount > 2 &&
        candidate == cyclePrevious
      )
    );

    cycleDestination = candidate;
    cycleProgress = 0.0f;
  }
}