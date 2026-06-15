// ============================================================
// ESP32 AI ASSISTANT WITH TFT DISPLAY AND JOYSTICK
// ============================================================
// PROJECT DESCRIPTION:
// This project uses an ESP32, ILI9341 TFT display, and joystick
// to create an interactive AI assistant system.
//
// FEATURES:
// 1. AI Assistant (Groq API)
// 2. Weather Information (WeatherAPI)
// 3. Calculator
// 4. Settings Screen
// 5. About Screen
//
// INPUTS:
// - Analog Joystick (X, Y)
// - Joystick Push Button
// - Serial Monitor Input
//
// OUTPUTS:
// - ILI9341 TFT Display
//
// EXECUTION FLOW:
// Start -> Initialize Hardware -> Connect WiFi ->
// Display Menu -> Read Joystick -> Execute Selected
// Function -> Display Results -> Return to Menu
//
// NOTE:
// Replace any real API keys with placeholders before
// uploading the repository publicly.
// ============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>


#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4


#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);


const char* ssid = "Wokwi-GUEST";
const char* password = "";


const char* apiKey = "PLACEHOLDER";


String menuItems[] = {
  "AI Assistant",
  "Weather",
  "Calculator",
  "Settings",
  "About"
};

int menuIndex = 0;

bool inMenu = true;
bool inAI = false;
bool inCalculator = false;

void showText(String text) {

  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  tft.println(text);
}

void drawMenu() {

  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.println("MAIN MENU");

  for (int i = 0; i < 5; i++) {

    tft.setCursor(10, 50 + (i * 30));

    if (i == menuIndex)
      tft.print("> ");
    else
      tft.print("  ");

    tft.println(menuItems[i]);
  }
}

void drawPage(String title) {

  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  tft.setCursor(10, 20);
  tft.println(title);

  tft.setTextSize(1);

  tft.setCursor(10, 80);
  tft.println("Press joystick");

  tft.setCursor(10, 95);
  tft.println("to return");
}

String askGroq(String question) {

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  if (!https.begin(
      client,
      "https://api.groq.com/openai/v1/chat/completions")) {

    return "Connection Failed";
  }

  https.addHeader(
      "Authorization",
      String("Bearer ") + apiKey);

  https.addHeader(
      "Content-Type",
      "application/json");

  DynamicJsonDocument doc(2048);

  doc["model"] = "llama-3.1-8b-instant";

  JsonArray messages =
      doc.createNestedArray("messages");

  JsonObject msg =
      messages.createNestedObject();

  msg["role"] = "user";
  msg["content"] =
      "Answer in less than 40 words: " +
      question;

  String payload;
  serializeJson(doc, payload);

  int httpCode = https.POST(payload);

  if (httpCode <= 0) {

    https.end();
    return "HTTP Error";
  }

  String response =
      https.getString();

  https.end();

  DynamicJsonDocument result(8192);

  if (deserializeJson(
          result,
          response)) {

    return "JSON Error";
  }

  return result["choices"][0]
               ["message"]
               ["content"]
      .as<String>();
}
String getWeather() {

  HTTPClient http;

  String url =
  "http://api.weatherapi.com/v1/current.json?key=a8145703d99040ecbeb201933261106&q=Manila";

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode <= 0) {
    http.end();
    return "Weather Error";
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(4096);

  if (deserializeJson(doc, payload)) {
    return "JSON Error";
  }

 String city =
  doc["location"]["name"].as<String>();

String temp =
  String(doc["current"]["temp_c"].as<float>());

String humidity =
  String(doc["current"]["humidity"].as<int>());

String condition =
  doc["current"]["condition"]["text"].as<String>();

return city +
       "\n" + condition +
       "\nTemp: " + temp + " C" +
       "\nHumidity: " + humidity + "%";
}



void setup() {

  Serial.begin(115200);

  pinMode(JOY_SW, INPUT_PULLUP);

  tft.begin();
  tft.setRotation(1);

  showText("Connecting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  drawMenu();

  Serial.println();
  Serial.println("Ready");
}

void loop() {

  if (inMenu) {

    int y = analogRead(JOY_Y);

    if (y < 1000) {

  menuIndex++;

  if (menuIndex > 4)
    menuIndex = 0;

  drawMenu();

  delay(200);
}

if (y > 3000) {

  menuIndex--;

  if (menuIndex < 0)
    menuIndex = 4;

  drawMenu();

  delay(200);
}

    if (digitalRead(JOY_SW) == LOW) {

      delay(200);

      switch (menuIndex) {

        case 0:

          inMenu = false;
          inAI = true;

          tft.fillScreen(
              ILI9341_BLACK);

          tft.setCursor(10, 10);
          tft.setTextSize(2);

          tft.println("AI Assistant");

          tft.setTextSize(1);

          tft.println();
          tft.println(
              "Type question in Serial");
          

          break;

        case 1: {

  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(10,10);
  tft.setTextSize(2);
  tft.println("Weather");

  tft.setTextSize(2);

  String weather = getWeather();

 tft.setCursor(10,50);
tft.println(weather);
  break;
}

        case 2:
          inMenu = false;
  inCalculator = true;

  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.println("Calculator");

  tft.setTextSize(2);
  tft.println("");
  tft.println("Type expression in Serial");
  
  tft.println("Example:");
  tft.println("5+3");

  Serial.println("Calculator Ready");

  break;

        case 3:
  

  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.println("Settings");

  tft.setTextSize(1);

  tft.setCursor(10, 50);
  tft.println("WiFi: Connected");

  tft.setCursor(10, 70);
  tft.println("IP:");

  tft.setCursor(40, 70);
  tft.println(WiFi.localIP());

  tft.setCursor(10, 90);
  tft.println("Signal:");

  tft.setCursor(60, 90);
  tft.print(WiFi.RSSI());
  tft.println(" dBm");

  tft.setCursor(10, 120);
  tft.println("Firmware: v1.0");

  tft.setCursor(10, 200);
  tft.println("Press joystick to return");

  break;

        case 4:
         tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.println("About");

  tft.setTextSize(1.9);

  tft.setCursor(10, 40);
 tft.println("Artificial Intelligence helps users access information quickly and efficiently. This project uses the ESP32 microcontroller to provide AI-powered assistance and useful features. It demonstrates the integration of modern AI technology with embedded systems for educational and practical applications.");

  tft.setCursor(10, 140);
  tft.println("Project Creator:");

  tft.setCursor(10, 155);
  tft.println("Romero Jolo M.");

  tft.setCursor(10, 180);
  tft.println("Papers Creator:");

  tft.setCursor(10, 195);
  tft.println("Harvey");

  break;
      }
    }
  }

  if (inAI) {

    if (Serial.available()) {

      String question =
          Serial.readStringUntil(
              '\n');

      question.trim();

      if (question.length() == 0)
        return;

      showText("Thinking...");

      String answer =
          askGroq(question);

      Serial.println();
      Serial.println(answer);

      showText(answer);
    }

    if (digitalRead(JOY_SW) == LOW) {

      delay(200);

      inAI = false;
      inMenu = true;

      drawMenu();
    }
  }
if (inCalculator) {

  if (Serial.available()) {

    String expr =
      Serial.readStringUntil('\n');

    expr.trim();

    float a, b, result;
    char op;

    sscanf(expr.c_str(),
           "%f%c%f",
           &a, &op, &b);

    switch (op) {

      case '+':
        result = a + b;
        break;

      case '-':
        result = a - b;
        break;

      case '*':
        result = a * b;
        break;

      case '/':
        if (b == 0) {
          showText("Divide by 0");
          return;
        }
        result = a / b;
        break;

      default:
        showText("Invalid");
        return;
    }

    showText("Result:\n" + String(result));
  }

  if (digitalRead(JOY_SW) == LOW) {

    delay(200);

    inCalculator = false;
    inMenu = true;

    drawMenu();
  }
}

}