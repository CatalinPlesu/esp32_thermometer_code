// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 15
#define RESTART_IF_FAILS 50
#define BLINK 30

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String temperatureC = "";

// Timer variables

// Replace with your network credentials
const char *ssid = "StarNet";
const char *password = "022844444";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC()
{
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00)
  {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  }
  else
  {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC);
  }
  return String(tempC);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <script>
    function loadPage() {
      fetch("https://catalinplesu.github.io/esp32_termometer/")
        .then(response => response.text())
        .then(html => {
          document.open();
          document.write(html);
          document.close();
        });
    }
  </script>
</head>
<body onload="loadPage()">
  <!-- contents of the original page will be replaced by contents of another-page.html before it loads -->
</body>
</html>
)rawliteral";

// Replaces placeholder with DS18B20 values
String processor(const String &var)
{
  // Serial.println(var);
  if (var == "TEMPERATUREC")
  {
    return temperatureC;
  }
  return String();
}
int unsigned fails = 0;

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();

  // Start up the DS18B20 library
  sensors.begin();

  temperatureC = readDSTemperatureC();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  pinMode(BUILTIN_LED, OUTPUT);
  while (WiFi.status() != WL_CONNECTED)
  {
    fails += 1;
    digitalWrite(BUILTIN_LED, HIGH); // sets the digital pin 13 on
    delay(BLINK);                      // waits for a second
    digitalWrite(BUILTIN_LED, LOW);  // sets the digital pin 13 off
    delay(BLINK);
    Serial.print(".");
    if (fails >= RESTART_IF_FAILS)
    {
      digitalWrite(BUILTIN_LED, HIGH); // sets the digital pin 13 on
      delay(1000);
      Serial.println("failed to connect restarting");
      ESP.restart();
    }
  }
  fails = 0;
  Serial.println();

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readDSTemperatureC().c_str()); });

  // Start server
  server.begin();
}

void loop()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    fails += 1;
    digitalWrite(BUILTIN_LED, HIGH); // sets the digital pin 13 on
    delay(BLINK);                      // waits for a second
    digitalWrite(BUILTIN_LED, LOW);  // sets the digital pin 13 off
    delay(BLINK);
    Serial.print(".");
    if (fails >= RESTART_IF_FAILS)
    {
      digitalWrite(BUILTIN_LED, HIGH); // sets the digital pin 13 on
      delay(1000);
      Serial.println("failed to connect restarting");
      ESP.restart();
    }
  }
}
