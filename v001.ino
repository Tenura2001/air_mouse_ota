#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <BleCombo.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>      
#include <up_down_inferencing.h> 
#include <Preferences.h>
#include "secrets.h"             

// ================== CONFIGURATION ==================
const char* github_owner = "ittipu";
const char* github_repo = "esp32_ota_update_private_repo";
const char* firmware_asset_name = "OTA_update_with_Github_Private_Repo.ino.bin";
const char* currentFirmwareVersion = "1.0.0"; 

#define SPEED 15                   
#define PIXEL_PIN 2
#define NUM_PIXELS 1
#define I2C_CLOCK_SPEED 400000     

#define EI_SAMPLING_FREQ_HZ 60
#define EI_SAMPLING_INTERVAL_MS (1000 / (EI_SAMPLING_FREQ_HZ + 1))
#define CONFIDENCE_THRESHOLD 0.75  

// Pins
#define BTN_SELECT 35 
#define BTN1       33 // Left
#define BTN2       27 // Right
#define BTN3       34
#define BTN4       32

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================== BITMAP ICONS (32x32) ==================
const unsigned char icon_mouse [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 
	0xff, 0x9f, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xb7, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 
	0xff, 0xbc, 0xff, 0xff, 0xff, 0xbe, 0x7f, 0xff, 0xff, 0xbf, 0x3f, 0xff, 0xff, 0xbf, 0x9f, 0xff, 
	0xff, 0xbf, 0xcf, 0xff, 0xff, 0xbf, 0xe7, 0xff, 0xff, 0xbf, 0xf3, 0xff, 0xff, 0xbf, 0xf9, 0xff, 
	0xff, 0xbf, 0xfc, 0xff, 0xff, 0xbf, 0xfe, 0x7f, 0xff, 0xbf, 0x80, 0x7f, 0xff, 0xbf, 0x9f, 0xff, 
	0xff, 0xb8, 0xdf, 0xff, 0xff, 0xb2, 0xdf, 0xff, 0xff, 0xa6, 0x6f, 0xff, 0xff, 0x8f, 0x6f, 0xff, 
	0xff, 0x9f, 0x27, 0xff, 0xff, 0xff, 0xb7, 0xff, 0xff, 0xff, 0xb3, 0xff, 0xff, 0xff, 0x9b, 0xff, 
	0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char icon_media [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 
	0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 
	0xef, 0xff, 0xff, 0xf7, 0xef, 0xf7, 0xff, 0xf7, 0xef, 0xf3, 0xff, 0xf7, 0xef, 0xf0, 0xff, 0xf7, 
	0xef, 0xf6, 0x3f, 0xf7, 0xef, 0xf7, 0x9f, 0xf7, 0xef, 0xf7, 0xc7, 0xf7, 0xef, 0xf7, 0xf3, 0xf7, 
	0xef, 0xf7, 0xf3, 0xf7, 0xef, 0xf7, 0xc7, 0xf7, 0xef, 0xf7, 0x9f, 0xf7, 0xef, 0xf6, 0x3f, 0xf7, 
	0xef, 0xf0, 0xff, 0xf7, 0xef, 0xf3, 0xff, 0xf7, 0xef, 0xf7, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 
	0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 
	0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char icon_ai [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0x87, 0xff, 
	0xff, 0xc3, 0xb7, 0xff, 0xff, 0x19, 0xb7, 0xff, 0xfc, 0x1d, 0x87, 0x1f, 0xf9, 0xfd, 0xfe, 0x4f, 
	0xf9, 0xfd, 0xfe, 0x4f, 0xf1, 0xfd, 0xfe, 0x0f, 0xe4, 0x7d, 0xfc, 0x3f, 0xee, 0x7d, 0xf9, 0xff, 
	0xef, 0xfd, 0x83, 0xff, 0xe3, 0xfd, 0xff, 0xff, 0xcf, 0xfd, 0xff, 0xc3, 0x9f, 0xfd, 0x80, 0x1b, 
	0x9f, 0xfd, 0x80, 0x1b, 0x9f, 0xbd, 0xff, 0xc3, 0xdf, 0x3d, 0xff, 0xff, 0xc6, 0x7d, 0x83, 0xff, 
	0xc0, 0xfd, 0xc1, 0xff, 0xcf, 0xfd, 0xfc, 0x3f, 0xef, 0xfd, 0xfe, 0x0f, 0xe7, 0xfd, 0xfe, 0x4f, 
	0xe3, 0xfd, 0xfe, 0x4f, 0xf0, 0xf9, 0x87, 0x1f, 0xfe, 0x79, 0xb7, 0xff, 0xff, 0x03, 0xb7, 0xff, 
	0xff, 0x8f, 0x87, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char icon_slides [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x01, 0x9f, 0xff, 0xff, 0xf9, 
	0x80, 0x00, 0x00, 0x01, 0xcf, 0xff, 0xff, 0xf3, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 
	0xef, 0xff, 0xff, 0xf7, 0xef, 0xf3, 0xff, 0xf7, 0xef, 0xf1, 0xff, 0xf7, 0xef, 0x14, 0x80, 0x37, 
	0xee, 0x10, 0xff, 0xf7, 0xee, 0xdf, 0xff, 0xf7, 0xec, 0xc3, 0x80, 0x77, 0xec, 0xf3, 0xff, 0xf7, 
	0xee, 0xf3, 0xff, 0xf7, 0xef, 0x07, 0xc0, 0x77, 0xef, 0xdf, 0xc0, 0x77, 0xef, 0xff, 0xff, 0xf7, 
	0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 0xef, 0xff, 0xff, 0xf7, 
	0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char icon_update [] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 
	0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 
	0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 
	0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 
	0xff, 0xfe, 0x7f, 0xff, 0xff, 0xce, 0x73, 0xff, 0xff, 0xe6, 0x67, 0xff, 0xff, 0xf2, 0x4f, 0xff, 
	0xff, 0xf8, 0x1f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0x9f, 0xff, 0xff, 0xf9, 
	0x9f, 0xff, 0xff, 0xf9, 0x9f, 0xff, 0xff, 0xf9, 0x9f, 0xff, 0xff, 0xf9, 0x9f, 0xff, 0xff, 0xf9, 
	0xcf, 0xff, 0xff, 0xf3, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char* menuIcons[] = {icon_mouse, icon_media, icon_ai, icon_slides, icon_update};

// ================== STATE MANAGEMENT ==================
enum AppState { STATE_MENU, STATE_RUNNING, STATE_OTA };
volatile AppState currentState = STATE_MENU;

enum SystemMode { MODE_MOUSE=0, MODE_MEDIA=1, MODE_MEDIA_AI=2, MODE_PRESENTATION=3 };
volatile int currentToolMode = MODE_MOUSE; 

const char* menuItems[] = {"Mouse Mode", "Media Control", "AI Control", "Slides", "Check Update"};
const int menuLength = 5;
int menuCursor = 0;
unsigned long modeStartTime = 0;

// ================== GLOBALS ==================
Adafruit_MPU6050 mpu;
Adafruit_NeoPixel pixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Preferences pref;

TaskHandle_t TaskCore0;     
SemaphoreHandle_t i2cMutex; 

volatile bool triggerInference = false; 
volatile bool isConnected = false;

volatile bool timerRunning = false;
volatile long timerRemaining = 600000; 
const long TIMER_START_VALUE = 600000; 
unsigned long lastBtnPress[5] = {0,0,0,0,0}; 
const int debounceDelay = 150; 

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;
unsigned long last_sample_time = 0;
unsigned long last_gesture_time = 0; 

// ================== DECLARATIONS ==================
void runOTASequence(); 
void configModeCallback(WiFiManager *myWiFiManager);
void checkForFirmwareUpdate();
void downloadAndApplyFirmware(String url);
void handleMenuInputs();
void handleToolInputs();
void drawMenu();
void drawToolStatus();
void setPixelColor(bool connected, int mode);
void displayAction(const char* msg);
void displayStatus(const char* msg);
void runInference();
void core0Task(void * pvParameters);
void ei_printf(const char *format, ...);

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  i2cMutex = xSemaphoreCreateMutex();

  Wire.begin();
  Wire.setClock(I2C_CLOCK_SPEED); 
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 Error"); for(;;); 
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  displayStatus("System Boot");

  pixel.begin();
  pixel.setBrightness(30); 
  pixel.setPixelColor(0, pixel.Color(10, 10, 10)); 
  pixel.show();

  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);

  pref.begin("app_state", false);
  bool startUpdate = pref.getBool("do_update", false);
  if (startUpdate) {
      pref.putBool("do_update", false);
      pref.end();
      pixel.setPixelColor(0, pixel.Color(0, 0, 255)); 
      pixel.show();
      runOTASequence(); 
  }
  pref.end();

  Keyboard.begin();
  Mouse.begin();

  if (!mpu.begin()) {
    displayStatus("MPU Error!");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); 

  xTaskCreatePinnedToCore(core0Task, "AI_Display", 10000, NULL, 1, &TaskCore0, 0);            
  delay(100);
}

// ================== LOOP ==================
void loop() {
  isConnected = Keyboard.isConnected(); 

  if (currentState == STATE_MENU) {
      handleMenuInputs();
  } 
  else if (currentState == STATE_RUNNING) {
      handleToolInputs(); 
      
      bool sensorNeeded = (currentToolMode == MODE_MOUSE || currentToolMode == MODE_MEDIA_AI);
      if (sensorNeeded && millis() > last_sample_time + EI_SAMPLING_INTERVAL_MS) {
        last_sample_time = millis();
        sensors_event_t a, g, temp;
        
        if (xSemaphoreTake(i2cMutex, (TickType_t) 5) == pdTRUE) {
          mpu.getEvent(&a, &g, &temp);
          xSemaphoreGive(i2cMutex); 

          if (isConnected) {
              if (currentToolMode == MODE_MOUSE) {
                  Mouse.move(g.gyro.z * -SPEED, g.gyro.y * SPEED);
                  feature_ix = 0; 
              } 
              else if (currentToolMode == MODE_MEDIA_AI) {
                  if (!triggerInference && feature_ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
                      features[feature_ix++] = a.acceleration.x;
                      features[feature_ix++] = a.acceleration.y;
                      features[feature_ix++] = a.acceleration.z;
                      if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) triggerInference = true; 
                  }
              }
          }
        } 
      }
  }
  delay(2); 
}

// ================== BACKGROUND TASK ==================
void core0Task(void * pvParameters) {
  static unsigned long lastScreenUpdate = 0;
  static unsigned long lastTimerCalc = 0;

  for(;;) {
    unsigned long currentMillis = millis();

    if (currentState == STATE_MENU) {
        if (currentMillis - lastScreenUpdate > 50) { 
            if (xSemaphoreTake(i2cMutex, (TickType_t) 50) == pdTRUE) {
                drawMenu();
                xSemaphoreGive(i2cMutex);
            }
            lastScreenUpdate = currentMillis;
        }
    }
    else if (currentState == STATE_RUNNING) {
        if (currentToolMode == MODE_PRESENTATION && timerRunning) {
            if (currentMillis - lastTimerCalc >= 1000) { 
                if (timerRemaining >= 1000) timerRemaining -= 1000;
                else { timerRemaining = 0; timerRunning = false; }
                lastTimerCalc = currentMillis;
            }
        }
        if (triggerInference && currentToolMode == MODE_MEDIA_AI) {
            runInference(); 
            feature_ix = 0; 
            triggerInference = false;
        }
        if (currentMillis - lastScreenUpdate > 100) { 
            setPixelColor(isConnected, currentToolMode);
            if (xSemaphoreTake(i2cMutex, (TickType_t) 50) == pdTRUE) {
                drawToolStatus();
                xSemaphoreGive(i2cMutex);
            }
            lastScreenUpdate = currentMillis;
        }
    }
    vTaskDelay(20 / portTICK_PERIOD_MS); 
  }
}

// ================== HORIZONTAL MENU LOGIC ==================
void handleMenuInputs() {
    unsigned long currentMillis = millis();
    
    // BTN1 = PREVIOUS (Left)
    if (digitalRead(BTN1) == LOW) {
        if (currentMillis - lastBtnPress[1] > debounceDelay) {
            menuCursor--; 
            if (menuCursor < 0) menuCursor = menuLength - 1; // Wrap around allowed? 
            // Or use: if (menuCursor < 0) menuCursor = 0; // Hard stop
            lastBtnPress[1] = currentMillis;
        }
    }
    // BTN2 = NEXT (Right)
    else if (digitalRead(BTN2) == LOW) {
        if (currentMillis - lastBtnPress[2] > debounceDelay) {
            menuCursor++; 
            if (menuCursor >= menuLength) menuCursor = 0; // Wrap around
            lastBtnPress[2] = currentMillis;
        }
    }
    // SELECT
    else if (digitalRead(BTN_SELECT) == LOW) {
         if (currentMillis - lastBtnPress[0] > 500) { 
             lastBtnPress[0] = currentMillis;
             if (menuCursor == 4) { 
                 displayStatus("Rebooting...");
                 pref.begin("app_state", false);
                 pref.putBool("do_update", true);
                 pref.end();
                 delay(500);
                 ESP.restart(); 
             } else {
                 currentToolMode = menuCursor;
                 feature_ix = 0; 
                 currentState = STATE_RUNNING;
                 modeStartTime = millis(); 
             }
         }
    }
}

void handleToolInputs() {
    unsigned long currentMillis = millis();

    if (digitalRead(BTN_SELECT) == LOW) {
        if (currentMillis - modeStartTime > 1000) {
            if (currentMillis - lastBtnPress[0] > 500) {
                currentState = STATE_MENU;
                lastBtnPress[0] = currentMillis;
                pixel.setPixelColor(0, pixel.Color(10, 10, 10)); 
                pixel.show();
                return; 
            }
        }
    }

    int btnPressed = -1;
    if (digitalRead(BTN1) == LOW) btnPressed = 1;
    else if (digitalRead(BTN2) == LOW) btnPressed = 2;
    else if (digitalRead(BTN3) == LOW) btnPressed = 3;
    else if (digitalRead(BTN4) == LOW) btnPressed = 4;

    if (btnPressed == -1 || currentMillis - lastBtnPress[btnPressed] < debounceDelay) return;
    lastBtnPress[btnPressed] = currentMillis;

    switch (currentToolMode) {
        case MODE_MOUSE:
            if (btnPressed == 1) Mouse.click(MOUSE_LEFT);
            if (btnPressed == 2) Mouse.click(MOUSE_RIGHT);
            if (btnPressed == 3) Mouse.move(0, 0, 1);
            if (btnPressed == 4) Mouse.move(0, 0, -1);
            break;
        case MODE_MEDIA:
        case MODE_MEDIA_AI:
            if (btnPressed == 1) Keyboard.write(KEY_MEDIA_PLAY_PAUSE);
            if (btnPressed == 2) Keyboard.write(KEY_MEDIA_NEXT_TRACK);
            if (btnPressed == 3) Keyboard.write(KEY_MEDIA_VOLUME_UP);
            if (btnPressed == 4) Keyboard.write(KEY_MEDIA_VOLUME_DOWN);
            break;
        case MODE_PRESENTATION:
            if (btnPressed == 1) Keyboard.write(KEY_RIGHT_ARROW);
            if (btnPressed == 2) Keyboard.write(KEY_LEFT_ARROW);
            if (btnPressed == 3) timerRunning = !timerRunning;
            if (btnPressed == 4) { timerRunning = false; timerRemaining = TIMER_START_VALUE; }
            break;
    }
}

// ================== HORIZONTAL DRAWING ==================
void drawMenu() {
    display.clearDisplay();

    // 1. Draw Left Arrow (Triangle pointing left)
    if (menuCursor > 0) {
        // x=5, y=32 (middle). Size ~10px
        display.fillTriangle(10, 32, 20, 22, 20, 42, SSD1306_WHITE);
    }

    // 2. Draw Icon (Centered)
    // 128 width. 32 icon width. Start X = 48.
    display.drawBitmap(48, 10, menuIcons[menuCursor], 32, 32, SSD1306_WHITE);

    // 3. Draw Right Arrow (Triangle pointing right)
    if (menuCursor < menuLength - 1) {
        // x=118, y=32.
        display.fillTriangle(118, 32, 108, 22, 108, 42, SSD1306_WHITE);
    }

    // 4. Draw Label (Centered below icon)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    String label = menuItems[menuCursor];
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 50);
    display.println(label);

    display.display();
}

void drawToolStatus() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(isConnected ? "BLE: ON" : "BLE: OFF");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    display.setCursor(0, 16);
    display.setTextSize(2);
    
    switch(currentToolMode) {
        case MODE_MOUSE: display.println("MOUSE"); display.setTextSize(1); display.setCursor(0, 40); display.println("Gyro Active"); break;
        case MODE_MEDIA: display.println("MEDIA"); display.setTextSize(1); display.setCursor(0, 40); display.println("Btns Only"); break;
        case MODE_MEDIA_AI: display.println("AI MODE"); display.setTextSize(1); display.setCursor(0, 40); display.println("Gestures ON"); break;
        case MODE_PRESENTATION:
            display.println("SLIDES");
            int minutes = timerRemaining / 60000;
            int seconds = (timerRemaining % 60000) / 1000;
            display.setTextSize(3); display.setCursor(10, 38);
            if (minutes < 10) display.print("0"); display.print(minutes); display.print(":");
            if (seconds < 10) display.print("0"); display.print(seconds);
            display.setTextSize(1); display.setCursor(90, 0); display.print(timerRunning ? "RUN" : "PAUSE");
            break;
    }
    display.display();
}

void setPixelColor(bool connected, int mode) {
    if (!connected) pixel.setPixelColor(0, pixel.Color(25, 0, 0)); 
    else {
        switch(mode) {
            case MODE_MOUSE: pixel.setPixelColor(0, pixel.Color(0, 0, 50)); break; 
            case MODE_MEDIA: pixel.setPixelColor(0, pixel.Color(0, 50, 0)); break; 
            case MODE_MEDIA_AI: pixel.setPixelColor(0, pixel.Color(30, 0, 30)); break; 
            case MODE_PRESENTATION: pixel.setPixelColor(0, pixel.Color(50, 30, 0)); break; 
        }
    }
    pixel.show();
}

void displayAction(const char* msg) {
    display.clearDisplay(); display.setCursor(0, 20); display.setTextSize(2); display.println(msg); display.display();
}

void displayStatus(const char* msg) {
  display.clearDisplay(); display.setCursor(0, 20); display.setTextSize(2); display.println(msg); display.display();
}

void configModeCallback(WiFiManager *myWiFiManager) {
  display.clearDisplay(); display.setCursor(0,0); display.setTextSize(1); display.println("Connection Failed");
  display.setCursor(0,15); display.println("Connect to WiFi:");
  display.setCursor(0,25); display.println("AirMouse_Setup");
  display.setCursor(0,35); display.println("Pass: 12341234");
  display.display();
}

void runOTASequence() {
  xSemaphoreTake(i2cMutex, portMAX_DELAY); 
  displayStatus("Connecting...");
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  WiFi.mode(WIFI_AP_STA); 
  wm.setConfigPortalTimeout(180); 

  if(!wm.autoConnect("AirMouse_Setup", "12341234")) {
     displayStatus("Connect Failed");
     delay(2000);
     ESP.restart(); 
  } 
  else {
     display.clearDisplay();
     display.setCursor(0, 0); display.println("WiFi Connected");
     display.setCursor(0, 20); display.println(WiFi.SSID());
     display.display();
     delay(2000);
     displayStatus("Checking FW...");
     checkForFirmwareUpdate();
  }
  displayStatus("Restarting...");
  delay(1000);
  ESP.restart();
}

void checkForFirmwareUpdate() {
  String apiUrl = "https://api.github.com/repos/" + String(github_owner) + "/" + String(github_repo) + "/releases/latest";
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); 
  
  http.begin(client, apiUrl);  
  http.addHeader("Authorization", "token " + String(github_pat));
  http.addHeader("Accept", "application/vnd.github.v3+json");
  http.setUserAgent("ESP32-OTA-Client");

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    display.clearDisplay();
    display.setCursor(0,0); display.println("API ERROR");
    display.setCursor(0,25); display.print("Code: "); display.println(httpCode);
    display.display(); delay(3000); http.end(); return;
  }

  StaticJsonDocument<4096> doc;
  deserializeJson(doc, http.getStream());
  http.end();

  String latestVersion = doc["tag_name"].as<String>();
  if (latestVersion != currentFirmwareVersion) {
    displayStatus("Updating...");
    JsonArray assets = doc["assets"].as<JsonArray>();
    for (JsonObject asset : assets) {
      if (asset["name"].as<String>() == String(firmware_asset_name)) {
        String assetId = asset["id"].as<String>();
        String firmwareUrl = "https://api.github.com/repos/" + String(github_owner) + "/" + String(github_repo) + "/releases/assets/" + assetId;
        downloadAndApplyFirmware(firmwareUrl);
        return;
      }
    }
    displayStatus("File Missing"); delay(2000);
  } else {
    displayStatus("Up to Date"); delay(2000);
  }
}

void downloadAndApplyFirmware(String url) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); 
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setUserAgent("ESP32-OTA-Client");
  http.begin(client, url);
  http.addHeader("Accept", "application/octet-stream");
  http.addHeader("Authorization", "token " + String(github_pat));

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) { http.end(); return; }
  int contentLength = http.getSize();
  if (!Update.begin(contentLength)) { http.end(); return; }

  WiFiClient* stream = http.getStreamPtr();
  uint8_t buff[1024];  
  size_t totalWritten = 0;
  int lastPct = 0;

  while (totalWritten < contentLength) {
    int available = stream->available();
    if (available > 0) {
      int readLen = stream->read(buff, min((size_t)available, sizeof(buff)));
      if (Update.write(buff, readLen) != readLen) { Update.abort(); http.end(); return; }
      totalWritten += readLen;
      int pct = (totalWritten * 100) / contentLength;
      if (pct - lastPct >= 10) {
          display.clearDisplay();
          display.setCursor(0, 20);
          display.print(pct); display.print("%");
          display.display();
          lastPct = pct;
      }
    }
    delay(1);
  }
  if (Update.end()) {
    displayStatus("Done! Restart");
    delay(1000);
    ESP.restart();
  } 
  http.end();
}

void runInference() {
    signal_t signal;
    ei_impulse_result_t result;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) return;
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, true);
    if (res != 0) return;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > CONFIDENCE_THRESHOLD) {
            if (millis() - last_gesture_time > 1000) { 
                if (strcmp(result.classification[ix].label, "up") == 0) {
                    Keyboard.write(KEY_MEDIA_VOLUME_UP);
                    if (xSemaphoreTake(i2cMutex, 100)) { displayAction("Vol UP"); xSemaphoreGive(i2cMutex); }
                    last_gesture_time = millis();
                }
                else if (strcmp(result.classification[ix].label, "down") == 0) {
                    Keyboard.write(KEY_MEDIA_VOLUME_DOWN);
                    if (xSemaphoreTake(i2cMutex, 100)) { displayAction("Vol DOWN"); xSemaphoreGive(i2cMutex); }
                    last_gesture_time = millis();
                }
            }
        }
    }
}
void ei_printf(const char *format, ...) {}
