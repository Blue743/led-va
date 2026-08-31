#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string.h>

const char* ssid = "asdf";
const char* password = "rockroll";

#define LED_PIN D1
#define NUM_LEDS 300
#define MAX_COLORS 8

ESP8266WebServer server(80);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);



// Stores one RGB color from the active JSON payload.
struct Color {
  int r;
  int g;
  int b;
};

// Stores the current visual mode and its active colors.
char currentMode[16] = "solid";
Color currentColors[MAX_COLORS];
int currentColorCount = 0;
int currentBrightness = 80;
unsigned long lastReconnectAttemptMs = 0;

void renderCurrentMode();


// =======
// BASE
// =======

// Fills the strip with a single RGB color.
void setColor(int r, int g, int b){
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// Uses the first active color as a solid fill.
void renderSolid() {
  if (currentColorCount < 1) {
    return;
  }

  setColor(
    currentColors[0].r,
    currentColors[0].g,
    currentColors[0].b
  );
}

// Splits the strip into sections using every active color.
void renderBands() {
  if (currentColorCount < 1) {
    return;
  }

  int section = NUM_LEDS / currentColorCount;
  if (section < 1) {
    section = 1;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    int colorIndex = i / section;
    if (colorIndex >= currentColorCount) {
      colorIndex = currentColorCount - 1;
    }

    strip.setPixelColor(
      i,
      strip.Color(
        currentColors[colorIndex].r,
        currentColors[colorIndex].g,
        currentColors[colorIndex].b
      )
    );
  }

  strip.show();
}

//millis related values for speed of animation.
unsigned long lastUpdateMs = 0;
const unsigned long updateIntervalMs = 1000;
unsigned long animationSpeed = 500;

void renderRandom() {

  if (currentColorCount < 1) {
  return;
  }

  unsigned long now = millis();

  if (now - lastUpdateMs < updateIntervalMs) {
    return;
  }

  lastUpdateMs = now;

  for (int i = 0; i < NUM_LEDS; i++) {

    int randIndex = random(currentColorCount);

     strip.setPixelColor(
      i,
      strip.Color(
        currentColors[randIndex].r,
        currentColors[randIndex].g,
        currentColors[randIndex].b
        )
      );
  }
  strip.show();
}

// Uses division to separate odd from even indexes on the strip.
void renderEach() {
  if (currentColorCount < 2) {
    return;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i % 2 == 0){
     strip.setPixelColor(
      i,
      strip.Color(
        currentColors[0].r,
        currentColors[0].g,
        currentColors[0].b
        )
      );
    }
    else{
      strip.setPixelColor(
      i,
      strip.Color(
        currentColors[1].r,
        currentColors[1].g,
        currentColors[1].b
        )
      );
    }
  }
  strip.show();
}

int ledIndex = 0;

void renderOne(){
  if (currentColorCount < 1) {
    return;
  }

  unsigned long timeNow = millis();

  if (timeNow - lastUpdateMs < updateIntervalMs) {
    return;
  }

  lastUpdateMs = timeNow;

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

int tail = 10;

void renderComet(){
  if (currentColorCount < 1){
    return;
  }

  unsigned long now = millis();

  if (now - lastUpdateMs < animationSpeed){
    return;
  }

  lastUpdateMs = now;

  for (int i = 0; i < tail; i++){

    int pixelIndex = ledIndex - i;
    int offLed = pixelIndex - tail -1;

    if (pixelIndex < 0){
      pixelIndex += NUM_LEDS;
    }

    float intensity = 1.0f - ((float) i / tail);

    int newR = currentColors[1].r * intensity;
    int newG = currentColors[1].g * intensity;
    int newB = currentColors[1].b * intensity;

    strip.setPixelColor(pixelIndex, strip.Color(
        newR,
        newG,
        newB
      )
    );

    if (offLed < 0){
      offLed += NUM_LEDS;
      }

    strip.setPixelColor(offLed, strip.Color(0,0,0)
    );
  }
  
  if (ledIndex >= NUM_LEDS){
    ledIndex = 0; 
  }

  ledIndex++;
  strip.show();
}

void renderAll(){
  if (currentColorCount < 1){
    return;
  }

  unsigned long timeNow = millis();

  if (timeNow - lastUpdateMs < updateIntervalMs) {
    return;
  }

  
  lastUpdateMs = timeNow;

  int randIndex = random(currentColorCount);
    
  for (int i =0; i <NUM_LEDS; i++){
    strip.setPixelColor(
      i,
      strip.Color(
        currentColors[randIndex].r,
        currentColors[randIndex].g,
        currentColors[randIndex].b
        )
      );
  }
  strip.show();
}

////// HANDLE JSON RESPONSE.
void handleApply(){
  if (!server.hasArg("plain")){
    server.send(400, "application/json", "{\"error\":\"Body missing\"}");
    return;
  }

  String body = server.arg("plain");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  const char* mode = doc["mode"];
  int brightness = doc["brightness"];
  JsonArray colors = doc["colors"];

  // Clear the previous color list before loading the new one.
  currentColorCount = 0;
  currentBrightness = brightness;
  if (mode) {
    strncpy(currentMode, mode, sizeof(currentMode) - 1);
    currentMode[sizeof(currentMode) - 1] = '\0';
  } else {
    strncpy(currentMode, "solid", sizeof(currentMode) - 1);
    currentMode[sizeof(currentMode) - 1] = '\0';
  }

  // Copy every JSON color into the active state array.
  for (size_t i = 0; i < colors.size(); i++) {
    if (currentColorCount >= MAX_COLORS) {
      break;
    }

    JsonArray color = colors[i];
    if (color.size() < 3) {
      continue;
    }

    currentColors[currentColorCount].r = color[0];
    currentColors[currentColorCount].g = color[1];
    currentColors[currentColorCount].b = color[2];
    currentColorCount++;
  }

  // Reject payloads that do not contain valid RGB colors.
  if (currentColorCount == 0) {
    server.send(400, "application/json", "{\"error\":\"No valid colors\"}");
    return;
  }

  // Save and apply the brightness from the latest request.
  currentBrightness = constrain(currentBrightness, 0, 255);
  strip.setBrightness(currentBrightness);

  // Apply the latest state immediately after receiving JSON.
  renderCurrentMode();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void renderCurrentMode() {
  if (strcmp(currentMode, "bands") == 0) {
    renderBands();
  } else if (strcmp(currentMode, "duo") == 0) {
    renderEach();
  } else if (strcmp(currentMode, "random") ==0) {
    renderRandom();
  } else if (strcmp(currentMode, "all") ==0) {
    renderAll();
  } else if (strcmp(currentMode, "comet") ==0) {
    renderComet();
  } else if (strcmp(currentMode, "one") ==0) {
    renderOne();
  } else {
    renderSolid();
  }
}

void handleRoot(){
  server.send(200, "text/plain", "ESP8266 online");
}

void handleBright(){
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "missing value");
    return;
  }

  int brightness = server.arg("value").toInt();
  currentBrightness = constrain(brightness, 0, 255);

  // Update the current brightness without replacing the active colors.
  strip.setBrightness(currentBrightness);
  strip.show();

  server.send(200, "text/plain", "bright");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void ensureWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();
  if (now - lastReconnectAttemptMs < 10000) {
    return;
  }

  lastReconnectAttemptMs = now;
  Serial.println("Wi-Fi disconnected. Reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
}

// SETUP

void setup(){
  Serial.begin(115200);
  strip.begin();
  strip.show();
  connectWiFi();
  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.on("/bright", handleBright);
  server.on("/apply", handleApply);
  server.begin();

  randomSeed(analogRead(A0));
}

// LOOP

void loop(){
  ensureWiFiConnection();
  ArduinoOTA.handle();
  server.handleClient();
  renderCurrentMode();
}
