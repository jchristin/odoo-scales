#include <WiFi.h>

const char *ssid = "";
const char *password = "";

WiFiServer server(80);
WiFiClient client;
String request;

String data;
String weight;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting sketch...");

  Serial1.begin(9600, SERIAL_8N1, D2, D3); // RX, TX
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi connection failed, retrying...");
    delay(1000);
  }

  Serial.println("WiFi connected.");

  Serial.print("Arduino Nano ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}

void writeCommonHeaders()
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println();
}

void writeOptionsHeaders()
{
  client.println("HTTP/1.1 204 No Content");
  client.println("Allow: OPTIONS, GET, HEAD, POST");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println();
}

void processRequest()
{
  Serial.print("Request: ");
  Serial.println(request);

  if (request.indexOf("GET /hw_proxy/hello") != -1) 
  {
    writeCommonHeaders();
  }

  if (request.indexOf("OPTIONS /hw_proxy/handshake") != -1) 
  {
    writeOptionsHeaders();
  }

  if (request.indexOf("POST /hw_proxy/handshake") != -1) 
  {
    writeCommonHeaders();
    client.println("{\"result\": true}");
  }

  if (request.indexOf("OPTIONS /hw_proxy/status_json") != -1) 
  {
    writeOptionsHeaders();
  }

  if (request.indexOf("POST /hw_proxy/status_json") != -1) 
  {
    writeCommonHeaders();
    client.println("{\"result\":{\"scale\":{\"status\":\"ok\"}}}");
  }

  if (request.indexOf("OPTIONS /hw_proxy/scale_read") != -1) 
  {
    writeOptionsHeaders();
  }

  if (request.indexOf("POST /hw_proxy/scale_read") != -1) 
  {
    writeCommonHeaders();
    client.print("{\"result\":{\"weight\":");
    client.print(weight);
    client.println("}}");
  }
}

void handleServer()
{
  if (client)
  {
    if (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        request += c;

        if (c == '\n')
        {
          processRequest();
          request="";
          client.stop();
          client = NULL;
        }
      }
    }
    else
    {
      client.stop();
      client = NULL;
    }
  }
  else
  {
    client = server.available();
  }
}

void extractWeight()
{
  Serial.print("Data: ");
  Serial.println(data);

  int index = data.indexOf('k');
  if (index != -1)
  {
    weight = data.substring(6, index);
  }
}

void handleSerial()
{
  if (Serial1.available()) {
    char receivedChar = Serial1.read();
    data += receivedChar;

    if (receivedChar == '\n')
    {
      extractWeight();
      data = "";
    }
  }
}

void loop()
{
  handleSerial();
  handleServer();
}