
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>
#include <Servo.h>

// Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Create aREST instance
aREST rest = aREST(client);

Servo servo_0;
Servo servo_1;
Servo servo_2;
Servo servo_3;

// Unique ID to identify the device for cloud.arest.io
char* device_id = "your device id";     // Ex. YYARM00

// WiFi parameters
const char* ssid = "your ssid";
const char* password = "your password";

// Variables to be exposed to the API
String local_ip = "";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Functions
void callback(char* topic, byte* payload, unsigned int length);

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int armControl(String msg){

   String data = getValue(msg, ' ', 0);
  int a0 = getValue(data, ',', 0).toInt();
  int a1 = getValue(data, ',', 1).toInt();
  int a2 = getValue(data, ',', 2).toInt();
  int a3 = getValue(data, ',', 3).toInt();

  Serial.println("armControl:");
  Serial.println("[" + msg + "]");
  Serial.println(a0);
  Serial.println(a1);
  Serial.println(a2);
  Serial.println(a3);

  servo_0.write(a0);
  servo_1.write(a1);
  servo_2.write(a2);
  servo_3.write(a3);

}

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Set callback
  client.setCallback(callback);

  // Init variables and expose them to REST API
  rest.variable("local_ip", &local_ip);

  rest.function("arm",armControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id(device_id);
  rest.set_name("YiyangArm");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Local server started on IP:");

  // Print the IP address
  Serial.println(WiFi.localIP());
  local_ip = ipToString(WiFi.localIP());


  servo_0.attach(5);  //D1
  servo_1.attach(4);  //D2  
  servo_2.attach(0);  //D3
  servo_3.attach(14);  //D5

}

void loop() {

  // Connect to the cloud
  rest.handle(client);

  // Handle Local aREST calls
  WiFiClient clientLocal = server.available();
  if (!clientLocal) {
    return;
  }
  while(!clientLocal.available()){
    delay(1);
  }
  rest.handle(clientLocal);

}

// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  rest.handle_callback(client, topic, payload, length);

}

// Convert IP address to String
String ipToString(IPAddress address)
{
  return String(address[0]) + "." +
    String(address[1]) + "." +
    String(address[2]) + "." +
    String(address[3]);
}
