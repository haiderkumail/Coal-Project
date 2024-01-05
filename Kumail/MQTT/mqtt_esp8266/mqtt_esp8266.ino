#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define RXp2 16
#define TXp2 17

// Replace the next variables with your SSID/Password combination
//const char* ssid = "HUAWEI-tbUR";
//const char* password = "3danj7qb";

const char* ssid = "Galaxy A30s3ABD";
const char* password = "fyqb1380";

bool isChanged = false;
bool newVal = 0;
const char* button1 = "testButton";
bool selfPublish = false;
bool skipChk = false;

String prevVal = "0";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "192.168.43.45";

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(button1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  String temp = Serial2.readString();
  int mid = 0;
  if (temp[1] == '-')
  {
    mid = 1;
  }
  else if (temp[2] == '-')
  {
    mid = 2;
  }
  else if (temp[3] == '-')
  {
    mid = 3;
  }
  String var1 = (temp.substring(0, mid));
  String var2 = temp.substring(mid+1, mid+2);

  Serial.println(var1);
  Serial.println(var2);

  int mappedValue = mapRange(var1.toInt(), 0, 255, 0, 1023);
  int v_input = mappedValue;
  float celVal = 125.315-0.175529*v_input-2.5;
  //var3 = var3 * 

  client.publish("100",  String(celVal).c_str());
  //client.publish("tryGraph",  String(mappedValue).c_str());

  if (isChanged == true)
  {
    isChanged = false;
    skipChk = true;
    if (newVal == 1)
    {
    Serial.println('A');
    Serial2.print('A');
    prevVal = "1";
    }
    else
    {
    Serial.println('B');
    Serial2.print('B');
    prevVal = "0";
    }
  }
  else
  {
    if (skipChk == true)
    {
      skipChk = false;
      return;
    }
    
    if (var2 != prevVal)
    {
      prevVal = var2;
    selfPublish = true;
    client.publish(button1, var2.c_str());
    }

  }
  /*
  else
  {
    //client.publish(button1, var2.c_str());
    isChanged = false;
    Serial.println(var2);
    Serial2.print(var2);
  }
  */
  
  
  //delay(2000);
}

int mapRange(int value, int oldMin, int oldMax, int newMin, int newMax) {
  // Check for division by zero
  if (oldMax == oldMin) {
    Serial.println("Error: oldMax and oldMin are the same.");
    return 0; // or handle the error in a way suitable for your application
  }

  // Perform the linear mapping
  return static_cast<int>((value - oldMin) * (newMax - newMin) / (oldMax - oldMin) + newMin);
}

void callback(char* topic, byte* message, unsigned int length) {
  if (selfPublish == true)
  {
    selfPublish = false;
    return;
  }
  /*
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  */
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == button1) {
    //Serial.println("here");
    isChanged = true;
    if (messageTemp == "1")
    {
      newVal = 1;
    }
    else
    {
      newVal = 0;
    }
  }
}