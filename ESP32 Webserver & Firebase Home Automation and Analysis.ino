#include <Vector.h>
//#include <Average.h>
#include "WiFi.h"
#include "FirebaseESP32.h"
#include "dht.h"

#define WIFI_SSID "VEDANT"
#define WIFI_PASSWORD "vedant3878"

#define FIREBASE_HOST "esptest2-8705c.firebaseio.com"
#define FIREBASE_AUTH "UOPCIJE9kNMBEqfgJ1xiOQ68AslWYg1a8BqcAa5W"

FirebaseData firebaseData;
FirebaseJson json;
dht DHT;
Vector<int>vector;
//Average<int>ave(200);

#define LED_BUILTIN 4
#define DHT11_PIN 2
#define LDR_PIN 15
#define Relay_PIN 12
#define LED 13

int Temperature = 0;
int Humidity = 0;
int temperatureCounter = 0;
int oldTemperatureCounter = 0;
//int ldrCounter = 0;
int testtime;
int testint;
int LDR = 0;
String relayStatus;
int n = 0;
String header;
String LEDstate = "on";
int chk;
int highestTemperature = 0;
int lowestTemperature = 0;
int pTemperature = 0;
int highestHumidity = 0;
int lowestHumidity = 0;
int pHumidity = 0;
int randomNumber;
double aveTemperature = 0;
double aveHumidity = 0;

WiFiServer server(80);

String star = "********************************************************************";

IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);


void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  pinMode(Relay_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected to IP : ");
  Serial.println(WiFi.localIP());
  Serial.println();
  server.begin();
  Serial.println("DHT11 Sensor Activated!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

}

void loop() {
  // put your main code here, to run repeatedly:
  webServer();
  funTemperature();
  funLDR();

  Firebase.setString(firebaseData, "/MyDatabase/Current Light Status", relayStatus);
  manualOverride();
  Firebase.setInt(firebaseData, "/MyDatabase/Current Temperature", Temperature);
  highestTemperature = (pTemperature > Temperature) ? pTemperature : Temperature;
  Firebase.setInt(firebaseData, "/MyDatabase/Highest Temperature", highestTemperature);
  highestHumidity = (pHumidity  > Humidity ) ? pHumidity : Humidity;
  Firebase.setInt(firebaseData, "/MyDatabase/Highest Humidity", highestHumidity);
  oldTemperatureCounter = temperatureCounter;
  temperatureCounter++;
  analysis(temperatureCounter);

  json.set("/Temperature", aveTemperature);
  json.set("/Humidity", aveHumidity);
  json.set("/LDR", LDR);
  Firebase.pushJSON(firebaseData, "/MyDatabase/JSON Storage", json);

  //timing();
}


void seperate(String star) {
  Serial.println(star);
}

void funTemperature() {
  chk = DHT.read11(DHT11_PIN);
  pTemperature = Temperature;
  Temperature = (int)DHT.temperature;
  pHumidity = Humidity;
  Humidity = (int)DHT.humidity;
  Serial.print("Temperature : ");
  Serial.println(Temperature);
  Serial.print("Humidity : ");
  Serial.println(Humidity);
}

void funLDR() {
  //int LDR = analogRead(LDR_PIN);
  //ldrCounter++;
  randomNumber = random(10, 200);
  LDR = randomNumber;
  if (LDR <= 200) {
    digitalWrite(Relay_PIN, LOW);
    relayStatus = "ON";
    Serial.print("LDR value : ");
    Serial.println(LDR);
    Serial.println("Lights turned on");
  } else {
    digitalWrite(Relay_PIN, HIGH);
    relayStatus = "OFF";
    Serial.print("LDR value : ");
    Serial.println(LDR);
    Serial.println("Lights turned off");
  }
}

int analysis(int temperatureCounter) {
  if (temperatureCounter == (oldTemperatureCounter + 1)) {

    /*
      average analysis
      file

    */
    vector.PushBack(temperatureCounter);
  }
  Serial.print("Data Entries : ");
  Serial.println(vector.Size());
  Serial.println(star);
  //  return;
}

void manualOverride() {
  if (Firebase.getString(firebaseData, "/MyDatabase/Current Light Status")) {

    if (firebaseData.dataType() == "string") {
      //  Serial.println("Datatype Matched");
      //   Serial.print("Firebase Data : ");
      //  Serial.println(firebaseData.stringData());

      if (relayStatus.equalsIgnoreCase("ON")) {
        digitalWrite(Relay_PIN, LOW);
        Serial.print("Firebase Sensor Status : ");
        Serial.println(firebaseData.stringData());
      } else
      {
        if (relayStatus.equalsIgnoreCase("OFF")) {
          digitalWrite(Relay_PIN, HIGH);
          Serial.print("Firebase Sensor Status : ");
          Serial.println(firebaseData.stringData());
        }
      }
    }
    else {
      Serial.println("Datatype did not Match");
      Serial.println(firebaseData.errorReason());
    }
  }
}

void timing() {
  if (Firebase.setTimestamp(firebaseData, "/MyDatabase/setTime")) {
    testtime = Firebase.getInt(firebaseData, "/MyDatabase/setTime");
  }
  Serial.print("testtime : ");
  Serial.println(testtime);
}


void webServer() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {

          if (currentLine.length() == 0) {

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /LED/on") >= 0) {
              Serial.println("GPIO LED on");
              LEDstate = "on";
              digitalWrite(LED, HIGH);
            } else if (header.indexOf("GET /LED/off") >= 0) {
              Serial.println("GPIO LED off");
              LEDstate = "off";
              digitalWrite(LED, LOW);
            }

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");

            client.println("<p>LED - State " + LEDstate + "</p>");

            if (LEDstate == "off") {
              client.println("<p><a href=\"/LED/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/LED/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");
            client.println();

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
