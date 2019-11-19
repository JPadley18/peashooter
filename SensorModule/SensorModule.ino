#include <dht11.h>
#include <WiFi101.h>

// Customise these attributes so that they fit your implementation
// ---------------------------------------------------------------
// The name of your WiFi
const char ssid[] = "WIFI_NAME";
// Your WiFi Password
const char pass[] = "YOUR_SECRET_PASSWORD";
// The ID of the device. If you are only making one then leave this as 1, but if you intend to use more than
// one device, you should give each one a unique numerical ID before uploading the sketch to each Arduino.
const byte plantId = 1;
// This is the hostname of your server running DSS, you can replace this with an IP address instead, and don't
// include the "http://" or "https://" prefix here.
const char hostName[] = "dss-server.example.com";
// This is the port that the Arduino will attempt to connect to. 443 is the default for a HTTPS, DSS server.
const int port = 443;
// ---------------------------------------------------------------

const int PROBE_PIN = A1;
const int LIGHT_PIN = A2;
const int DHT_PIN = A3;
const int CONNECTION_WAIT = 10;
const char contentType[] = "application/json";
const char postTemplate[] = "{'plant_id': %d, 'sensor_moisture': %d, 'sensor_light': %d, 'sensor_temperature': %d, 'sensor_humidity': %d}";

int status = WL_IDLE_STATUS;

WiFiClient client;
dht11 dht;

void setup() {
  Serial.begin(9600);

  if(WiFi.status() == WL_NO_SHIELD){
    Serial.println("No WiFi Shield");
    while (true);
  }

  while(status != WL_CONNECTED){
    Serial.println("Attempting to connect to WiFi...");
    status = WiFi.begin(ssid, pass);
    delay(CONNECTION_WAIT * 1000);
  }

  printWiFiStatus();
  
  pinMode(PROBE_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  dht.read(DHT_PIN);

  int moisture = analogRead(PROBE_PIN);
  int light = analogRead(LIGHT_PIN);
  int temperature = dht.temperature;
  int humidity = dht.humidity;

  char buf[sizeof(postTemplate) + 20];
  sprintf(&buf[0], postTemplate, plantId, moisture, light, temperature, humidity);
  String postData = String(buf);
  
  if(client.connectSSL(hostName, port)){
    client.println("POST /public/api/v1/new_measurements/insert_new/query HTTP/1.1");
    client.print("Host: ");
    client.println(hostName);
    client.print("Content-Type: ");
    client.println(contentType);
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
    Serial.println("API Call Successful");
  } else{
    Serial.println("Failed to Connect to Server!");
  }

  client.stop();

  delay(300000);
}
