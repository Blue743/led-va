#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string.h>
#include <secrets.h>

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

//millis related values for speed of animation.
unsigned long lastUpdateMs = 0;
const unsigned long updateIntervalMs = 1000;
unsigned long animationSpeed = 25;
unsigned long fadeSpeed = 100;
unsigned long lastFadeUpdate = 50;
unsigned long lastSpawn = 500;

//valores relacionados a lógica de animação
int ledIndex = 0;
int tail = 30;
int partitions = 5;
int partitionCounter = 0;
int lastColor = -1;
float progress = 0.0f;
float progressStep = 0.05f;
int randIndex = 1;

//valores de pole
int colorIndex = 0;

//god help me (sakura/cherry)
int destinationColor;
int originColor;
int previousColor;
bool cycleStarted = false;
bool sakuraCycle = false;

//night function
bool nightCycle = false;
int stars[11];
int starPosition = 0;
float starProgress[11];
float starProgressStep[11];


void setColor(int r, int g, int b){
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

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

void renderGroups() {
  if (currentColorCount < 1){
    return;
  }

  unsigned long now = millis();

  if (now - lastUpdateMs < updateIntervalMs){
    return;
  }

  lastUpdateMs = now;

  int section = NUM_LEDS / partitions; // section é 60
  int start = partitionCounter * section;
  int end = start + section;

  int randIndex = random(currentColorCount);

  while (randIndex == lastColor && currentColorCount > 1)
  {
    randIndex = random(currentColorCount);
  }

  lastColor = randIndex;

  for (int i = start; i < end; i++){
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
    partitionCounter++;

    if (end >= NUM_LEDS){
        partitionCounter = 0;
      }
}

void renderNight(){

  if (currentColorCount < 2){
    return;
  }


  if (!nightCycle){
    for (int i = 0; i < 11; i++){
      stars[i] = random(NUM_LEDS); // SORTEIA OS INDEXES ONDE VAO NASCER AS ESTRELAS
      starProgress[i] = 0.0f; // INICIALIZA TODAS ELAS NO 0.0
      starProgressStep[i] = 0.1f;
    }

    nightCycle = true; // TRAVA PARA SÓ RODAR UMA VEZ. 
  } 

  unsigned long now = millis();

  if (now - lastUpdateMs < fadeSpeed){
    return;
  }

  lastUpdateMs = now; 

  for (int i = 0; i < NUM_LEDS; i++){
    strip.setPixelColor(
      i, 
       strip.Color(  //APLICA A TINTA NO INDICE DA VOLTA ATUAL.
        currentColors[0].r,
        currentColors[0].g,  // PINTA O FUNDO DE AZUL, ANTES DE TUDO.
        currentColors[0].b
      )
    );
  }

  for (int i = 0; i < 11; i++){

    //CALCULO FEITO DENTRO DO FOR, PRA PODER ACOMPANHAR TODOS OS ESTADOS A CADA QUADRO QUE RODAR. 
    //A CADA VOLTA DO FOR ELE PEGA starProgress[i] E CALCULA INDIVIDUALMENTE O PROGRESSO.
    int newR = currentColors[0].r + (currentColors[1].r - currentColors[0].r) * starProgress[i];
    int newG = currentColors[0].g + (currentColors[1].g - currentColors[0].g) * starProgress[i];
    int newB = currentColors[0].b + (currentColors[1].b - currentColors[0].b) * starProgress[i];

    strip.setPixelColor(
      stars[i], 
       strip.Color(  //APLICA A TINTA NO INDICE DA VOLTA ATUAL.
        newR,
        newG,
        newB
      )
    );

    starProgress[i] += starProgressStep[i]; // SOMA 0.5 EM CIMA DO AVANÇO ATUAL, SENDO 0.0 O INICIO E 1.0 COR COMPLETA.

  if (starProgress[i] >= 1.0f && starProgressStep[i] >= 0.0f){ // CHECKA SE A ESTRELA ATUAL ESTÁ FINALIZADA.
    starProgress[i] = 1.0f; // DEFINE 1.0 PARA QUE NAO ULTRAPASSE E CALCULE VALORES RBG INVÁLIDOS (ACIMA DE 255)
    starProgressStep[i] = -starProgressStep[i]; // NEGATIVA PROGRESS STEP, PARA QUE DE 1.0 ELE DESÇA PARA 0.0, CRIANDO O FADE DE VAI E VEM.
  }

  if (starProgress[i] <=0.0f && starProgressStep[i] <= 0.0f){  //VERIFICA SE TERMINOU DE DESCER, SE TERMINOU, COMEÇA A SUBIR.
    starProgress[i] = 0.0f;
    starProgressStep[i] = -starProgressStep[i]; 
  }

    if (starProgress[i] <= 0.0f){ 
      stars[i] = random(NUM_LEDS);
      starProgressStep[i] = random(1,11) / 100.0f;
  }

  }
    strip.show();
}

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

void renderOne(){
  if (currentColorCount < 1) {
    return;
  }

  unsigned long timeNow = millis();

  if (timeNow - lastUpdateMs < fadeSpeed) {
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

void renderComet() {
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

    int newR = currentColors[0].r * intensity;
    int newG = currentColors[0].g * intensity;
    int newB = currentColors[0].b * intensity;

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

void renderSakura(){
  if (currentColorCount < 2){
    return;
  }

  unsigned long now = millis();


  if (!sakuraCycle){
    originColor = random(currentColorCount);

    do {
      destinationColor = random(currentColorCount);
    } while (originColor == destinationColor);

    sakuraCycle = true;

  }


  if (now - lastUpdateMs < fadeSpeed){
    return;
  }

  lastUpdateMs = now;

  if (progress >= 1.0f){
    progress = 1.0f;
  }

  int newR = currentColors[originColor].r + (currentColors[destinationColor].r - currentColors[originColor].r) * progress;
  int newG = currentColors[originColor].g + (currentColors[destinationColor].g - currentColors[originColor].g) * progress;
  int newB = currentColors[originColor].b + (currentColors[destinationColor].b - currentColors[originColor].b) * progress;

  for (int i = 0; i < NUM_LEDS; i++){
    strip.setPixelColor(
      i,
       strip.Color(
        newR,
        newG,
        newB
      )
    );
  }

  strip.show();

  if (progress >= 1.0f){
    previousColor = originColor;
    originColor = destinationColor;

    int candidate;

    do {
      candidate = random(currentColorCount);
    }while(candidate == originColor || candidate == previousColor);

    destinationColor = candidate;
    progress = 0.0;

  } else {
    progress += progressStep;
  }
}

void renderColorCycle(){

  if (currentColorCount < 2) {
    return;
  }

  if (!cycleStarted){ // se o cyclo startou 
    originColor = random(currentColorCount); // sorteia um index para preencher o origin

    do { 
      destinationColor = random(currentColorCount);  // << esse bloco sorteia continuamente até que o destination não seja identico ao origin.
    } while(originColor == destinationColor);

    cycleStarted = true;

  }

  unsigned long now = millis(); // executa o millis pra saber a quanto tempo o arduino está ligado.

  if (now - lastUpdateMs < fadeSpeed){ // faz a validação se o tempo agora - o tempo de update é menor que o fadespeed, se for, retorna.
    return;
  }

  lastUpdateMs = now; // atualiza o horario pra informar a validação acima.

  if (progress >= 1.0f){
    progress = 0.0;
  }

  int newR = currentColors[originColor].r + (currentColors[destinationColor].r - currentColors[originColor].r)  * progress;
  int newG = currentColors[originColor].g + (currentColors[destinationColor].g - currentColors[originColor].g)  * progress;  // calculo newRGB's.
  int newB = currentColors[originColor].b + (currentColors[destinationColor].b - currentColors[originColor].b)  * progress;

  for(int i = 0; i < NUM_LEDS; i++)
  strip.setPixelColor(
    i,
     strip.Color(      // << for itera sobre a fita e pinta ela com os novos valores calculados acima com os newRGB's.
      newR,
      newG,
      newB
    )
  );

  strip.show(); // envia o comando pra fita ligar.

  if (progress >= 1.0f){  // trava pra nao extrapolar acima de 1.0 e fazer calculo errado.

    previousColor = originColor; // guarda a previous color como a cor antiga de origem.
    originColor = destinationColor; // em seguida guarda a ultima destination na origem.

    int candidate; 

    do {
      candidate = random(currentColorCount); // sorteia um index até que ele nao seja identico a ultima cor usada, previous, ou identico a cor que já está, origem
    } while( candidate == originColor || candidate == previousColor);

    destinationColor = candidate;
    progress = 0.0; // mantem ele no 0.0 para proxima viagem.
  } else {
  progress += progressStep;
  }

}

void renderCold() {

  if (currentColorCount < 2 ){
    return;
  }

  unsigned long now = millis();

  if (now - lastUpdateMs < fadeSpeed){
    return;
  }

  lastUpdateMs = now;

  if (progress >= 1.0f && progressStep >= 0.0f){
    progress = 1.0f;
    progressStep = -progressStep;
  }

  if(progress <= 0.0f && progressStep <= 0.0f){
    progress = 0.0f;
    progressStep = -progressStep;
  }

  int newR = currentColors[0].r + (currentColors[1].r - currentColors[0].r) * progress;
  int newG = currentColors[0].g + (currentColors[1].g - currentColors[0].g) * progress;
  int newB = currentColors[0].b + (currentColors[1].b - currentColors[0].b) * progress;

  for (int i = 0; i < NUM_LEDS; i++){

    strip.setPixelColor(i, strip.Color(
        newR,
        newG,
        newB
      )
    );
  }

  progress +=progressStep;
  strip.show();
}

void renderPole() {

  if (currentColorCount < 1){
    return;
  }

  unsigned long now = millis();

  if ( now - lastUpdateMs < fadeSpeed){
    return;
  }

  lastUpdateMs = now;

  for (int i = 0; i < NUM_LEDS; i++){
    strip.setPixelColor(
      i,
      strip.Color(
        currentColors[colorIndex].r,
        currentColors[colorIndex].g,       
        currentColors[colorIndex].b
      )
    );

    if(colorIndex > 2){
      colorIndex = 0;
    }
    
    colorIndex++;
  }

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
    
  for (int i =0; i < NUM_LEDS; i++){
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

void renderOff(){
  for (int i = 0; i < NUM_LEDS; i++){
    strip.setPixelColor(i, strip.Color(0,0,0));
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
  } else if (strcmp(currentMode, "sakura") ==0) {
    renderSakura();
  } else if (strcmp(currentMode, "group") ==0) {
    renderGroups();
  } else if (strcmp(currentMode, "cycle") ==0) {
    renderColorCycle();
  } else if (strcmp(currentMode, "pole") ==0) {
    renderPole();
  } else if (strcmp(currentMode, "cold") ==0) {
    renderCold();
  } else if (strcmp(currentMode, "galaxy") ==0){
    renderNight();
  } else if (strcmp(currentMode, "one") ==0) {
    renderOne();
  } else if (strcmp(currentMode, "off") ==0){
    renderOff();
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
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
