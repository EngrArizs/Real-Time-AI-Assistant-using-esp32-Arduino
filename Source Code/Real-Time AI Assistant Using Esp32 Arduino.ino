#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans9pt7b.h>

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
String typingText = "";
String calcText = "";

void showText(String text) {

  tft.fillScreen(ILI9341_BLACK);

  tft.setFont();                 
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println(text);
}

void drawMenu() {

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);

  
  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(1);

  
  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds("MAIN MENU", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((320 - w) / 2, 32);
  tft.print("MAIN MENU");

  
  int y = 70;

  for (int i = 0; i < 5; i++) {

    tft.setCursor(25, y);

    if (i == menuIndex)
      tft.print("> ");
    else
      tft.print("  ");

    tft.print(menuItems[i]);

    y += 28;
  }

  
  tft.setFont();
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
    "http://api.weatherapi.com/v1/current.json?key=PLACEHOLDER&q=Tacurong";

  http.begin(url);

  int code = http.GET();

  if (code <= 0) {
    http.end();
    return "HTTP Error";
  }

  String weatherData = http.getString();
  http.end();

  Serial.println(weatherData);

  DynamicJsonDocument doc(8192);

  DeserializationError error =
      deserializeJson(doc, weatherData);

  if (error) {
    Serial.println(error.c_str());
    return "JSON Error";
  }

  String city =
      doc["location"]["name"].as<String>();

  String condition =
      doc["current"]["condition"]["text"].as<String>();

  float temp =
      doc["current"]["temp_c"].as<float>();

  int humidity =
      doc["current"]["humidity"].as<int>();

  return city +
         "\n" + condition +
         "\nTemp: " + String(temp, 1) + " C" +
         "\nHumidity: " + String(humidity) + "%";
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

         tft.setFont(&FreeSans9pt7b);
tft.setTextSize(1);

int16_t x1, y1;
uint16_t w, h;

tft.getTextBounds("AI Assistant", 0, 0, &x1, &y1, &w, &h);
tft.setCursor((320 - w) / 2, 30);
tft.print("AI Assistant");

          tft.setTextSize(1);

          tft.println("");
          tft.println(
              "Type question in Serial");
          

          break;

        case 1: {

  tft.fillScreen(ILI9341_BLACK);

  tft.setFont(&FreeSans9pt7b);
tft.setTextSize(1);


tft.setCursor(78, 20);
tft.println("Weather");

String weather = getWeather();


tft.setCursor(20, 60);
tft.println(weather);


tft.setCursor(20, 210);
tft.println("Press joystick to return");


tft.setFont();

break;
}

      case 2:
          inMenu = false;
  inCalculator = true;

  tft.fillScreen(ILI9341_BLACK);

  tft.setFont(&FreeSans9pt7b);
tft.setTextSize(1);
tft.setCursor(55, 30);
tft.print("Calculator");

  tft.setTextSize(1);
  tft.println("");
  tft.println("Type expression in Serial");
  
  tft.println("Example:");
  tft.println("5+3");

  Serial.println("Calculator Ready");

  break;

        case 3:
  

 tft.fillScreen(ILI9341_BLACK);
tft.setTextColor(ILI9341_WHITE);


tft.setFont(&FreeSans9pt7b);
tft.setTextSize(1);
tft.setCursor(55, 30);
tft.print("Settings");


tft.setCursor(10, 60);
tft.print("WiFi: Connected");

tft.setCursor(10, 90);
tft.print("IP: ");
tft.print(WiFi.localIP());

tft.setCursor(10, 120);
tft.print("Signal: ");
tft.print(WiFi.RSSI());
tft.print(" dBm");

tft.setCursor(10, 150);
tft.print("Firmware: v1.0");


tft.setCursor(10, 215);
tft.print("Press joystick to return");


tft.setFont();

break;

        case 4:
       
 tft.fillScreen(ILI9341_BLACK);
tft.setTextColor(ILI9341_WHITE);


tft.setFont();
tft.setFont(&FreeSans9pt7b);
tft.setTextSize(2);
tft.setCursor(55, 30);
tft.println("About");


tft.setFont(&FreeSans9pt7b);
tft.setTextSize(1);

tft.setCursor(10, 50);   tft.print("Artificial Intelligence helps");
tft.setCursor(10, 65);   tft.print("users access information");
tft.setCursor(10, 80);   tft.print("quickly and efficiently.");
tft.setCursor(10, 95);   tft.print("This project uses the");
tft.setCursor(10, 110);  tft.print("ESP32 microcontroller");
tft.setCursor(10, 125);  tft.print("to provide AI-powered");
tft.setCursor(10, 140);  tft.print("assistance and useful");
tft.setCursor(10, 155);  tft.print("features.");


tft.setCursor(10, 180);  tft.print("Project: Romero Jolo M.");
tft.setCursor(10, 200);  tft.print("Paper: Harvey");


tft.setFont();

break;
      }
    }
  }

if (inAI) {

  while (Serial.available()) {

    char c = Serial.read();

    if (c == 8 || c == 127) {

      if (typingText.length() > 0) {

        typingText.remove(typingText.length() - 1);

        Serial.print("\b \b");
      }
    }
    else if (c == '\n' || c == '\r') {

      Serial.println();

      if (typingText.length() > 0) {

        showText("Thinking...");

        String answer = askGroq(typingText);

        Serial.println("AI:");
        Serial.println(answer);

        showText(answer);

        typingText = "";
      }
    }
    else {

      typingText += c;

      Serial.print(c);
    }
  }

  if (digitalRead(JOY_SW) == LOW) {

    delay(200);

    inAI = false;
    inMenu = true;

    typingText = "";

    drawMenu();
  }
}
if (inCalculator) {

  while (Serial.available()) {

    char c = Serial.read();

    if (c == 8 || c == 127) { 

      if (calcText.length() > 0) {
        calcText.remove(calcText.length() - 1);
        Serial.print("\b \b");
      }

    } else if (c == '\n' || c == '\r') {

      Serial.println();

      if (calcText.length() > 0) {

        float a, b, result;
        char op;

        if (sscanf(calcText.c_str(), "%f%c%f", &a, &op, &b) != 3) {

          showText("Invalid");
          calcText = "";
          continue;
        }

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
              calcText = "";
              continue;
            }

            result = a / b;
            break;

          default:
            showText("Invalid");
            calcText = "";
            continue;
        }

        Serial.print("Result: ");
        Serial.println(result);

        showText("Result:\n" + String(result));

        calcText = "";
      }

    } else {

      calcText += c;
      Serial.print(c);  
    }
  }

  if (digitalRead(JOY_SW) == LOW) {

    delay(200);

    inCalculator = false;
    inMenu = true;

    calcText = "";

         
    tft.setTextSize(1);  

    drawMenu();
  }
}
}