#include "heltec.h"
#include "WiFi.h"
#include "images.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "Arduino.h"

#define tonehz 600       //the approximate frequency of the tones in hz, in reality it will be a tad lower, and includes lots of harmonics.
#define dit 64           //the length of the dit in milliseconds. The dah and pauses are derived from this.
#define rest 30000       //the amount of time between transmits in ms 60000=60seconds
#define longlength 10000 //length of long tone in milliseconds 10000=10seconds
//#define tx 12             //pin 12 on the board controls the tx, signal to 5v relay where you would have speaker/mic pins connected
//#define audio 14          //the pin 14 on the board that controls audio output, connected to mic on radio
const int tx = 26;
const int audio = 27;

//CW array DO NOT CHANGE
// 1 = dit , 2 = dah, 0 = space
// Letters/#                 A     B      C     D    E    F      G     H      I     J     K      L    M     N    O     P      Q      R     S    T    U      V     W      X     Y      Z       0        1      2       3       4       5       6       7       8       9
String morseTable[] = {"0", "12", "2111", "2121", "211", "1", "1121", "221", "1111", "11", "1222", "212", "1211", "22", "21", "222", "1221", "2212", "121", "111", "2", "112", "1112", "122", "2112", "2122", "2211", "22222", "12222", "11222", "11122", "11112", "11111", "21111", "22111", "22211", "22221"};

// Declaring Function prototypes
String formMorse(String input);
void playcode(String input);
void playtone(int note_duration);

// Replace with your network credentials
const char* ssid = "foxhunt";
const char* password = "";
 
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
 
// Search for parameters in HTTP POST request
const char* PARAM_INPUT_1 = "transmit";
const char* PARAM_INPUT_2 = "call";
 
// Variables to save values from HTML form
String transmit;
String call;
String sendmorse = "";
// Change to your call sign and text you want to announces
String Text = "";                   //This is what will be transmitted will be created from call variable later in code

// Global Varibables
int TextChars = 15;
int CodeChars;
int duration;
int note = 1000000 / tonehz; //converts the frequency into period
 
// Variable to detect whether a new request occurred
bool newRequest = false;
 
// HTML to build the web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Fox Hunt Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Fox Hunt Config</h1>
    <form action="/" method="POST">
      <input type="radio" name="transmit" value="ON">
      <label for="ON">Start</label>
      <input type="radio" name="transmit" value="OFF">
      <label for="ON">Stop</label><br><br><br>
      <label for="call">Callsign to use:</label>
      <input type="text" name="call">
      <input type="submit" value="Hunt!">
    </form>
</body>
</html>
)rawliteral";
 
// Initialize WiFi
void initWiFi() {
  WiFi.softAP(ssid, password);
  IPAddress local_IP(192,168,4,1);
  //Serial.println(WiFi.SoftAPIP());
}
 
void logo(){
  Heltec.display -> clear();
  Heltec.display -> drawXbm(0,5,fox_width,fox_height,(const unsigned char *)fox_bits);
  Heltec.display -> display();
  delay(2000);
}
 
void setup() {
  Serial.begin(115200);
  pinMode(tx, OUTPUT);
  //pinMode(10, INPUT);
  pinMode(audio, OUTPUT);
  pinMode(LED, OUTPUT);     
  digitalWrite(tx, LOW);
  initWiFi();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
   
  // Handle request (form)
  Serial.println("Starting Server");
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST input1 value (direction)
        if (p->name() == PARAM_INPUT_1) {
          transmit = p->value().c_str();
          Serial.print("transmit set to: ");
          Serial.println(transmit);
        }
        // HTTP POST input2 value (steps)
        if (p->name() == PARAM_INPUT_2) {
          call = p->value().c_str();
          Serial.print("callsign set to: ");
          Serial.println(call);
        }
      }
    }
    request->send(200, "text/html", index_html);
    newRequest = true;
  });
 
  server.begin();
  //display splash screen and instructions
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, false /*Serial Enable*/);
  logo();
  delay(2000);
  Heltec.display -> clear();
  Heltec.display -> setFont(ArialMT_Plain_16);
  Heltec.display -> drawString(0, 0, "SSID foxhunt");
  Heltec.display -> drawString(0, 16, "No Password");
  Heltec.display -> drawString(0, 32, "http://192.168.4.1");
  //Heltec.display -> drawString(0, 30, "192.168.4.1");
  Heltec.display -> display();
}
 
void loop() {
  // Check if there was a new request and move the stepper accordingly
  if (newRequest){
    if (transmit == "ON"){
      sendmorse = "ON";
      delay(500);
      Heltec.display -> clear();
      Heltec.display -> setFont(ArialMT_Plain_16);
      Heltec.display -> drawString(0, 16, "Callsign: " + (call));
      Heltec.display -> drawString(0, 32, "TX Enable");
      Heltec.display -> display();
      delay(500);
    }
    else{
      sendmorse = "OFF";
      delay(500);
      Heltec.display -> clear();
      Heltec.display -> setFont(ArialMT_Plain_16);
      //Heltec.display -> drawString(0, 0, "Callsign: " + (call));
      Heltec.display -> drawString(0, 16, "TX Disabled");
      Heltec.display -> display();
      delay(500);
    }
    newRequest = false;
  }
  if (sendmorse == "ON"){
      String Text = call + " FOX " + call + " FOX ";
      String code = formMorse(Text);
      digitalWrite(tx, HIGH);     //starts the radio transmitting
      playtone(longlength);
      delay(250);
      playcode(code);
      Serial.println(Text);
      Serial.println(code);
      digitalWrite(tx, LOW); //Stops the radio's transmission
      delay(rest);           // Delay in transmissions
  }
}

//   Function definition: playtone
void playtone(int note_duration) {
  long elapsed_time = 0;
  long startTime = millis();
  if (note_duration > 0) {
    digitalWrite(LED, HIGH);                  //See when it is making a tone on the led
    while (elapsed_time < note_duration) {
      digitalWrite(audio, HIGH);
      delayMicroseconds(note / 2);
      digitalWrite(audio, LOW);
      delayMicroseconds(note / 2);
      elapsed_time = millis() - startTime;
    }
    digitalWrite(LED, LOW);
  }
  else { //if it's a pause this will run

    delay(dit * 2);
  }
} // End of Function "playtone"


//   Function definition: playcode
void playcode(String input) {
  for (int i = 0; i < input.length(); i++) {
    Serial.print(input[i]);
    if (input[i] == '0') { //See if it's a pause
      duration = 0;
    }
    else if (input[i] == '1') { //See if it's a dit
      duration = dit;
    }
    else if (input[i] == '2') { //See if it's a dah
      duration = dit * 3;
    }
    playtone(duration);
    delay(dit); //makes a pause between sounds, otherwise each letter would be continuous.
  }
  Serial.println();
} // End of Function "playcode"


//   Function definition: formMorse
String formMorse(String input) {
  input.toUpperCase();
  String output = "";
  for (int i = 0; i < input.length() ; i++) {
    if (input[i] >= 65 && input[i] <= 90)
      output = output + morseTable[input[i] - 64] + '0';
    else if (input[i] >= 48 && input[i] <= 57)
      output = output + morseTable[input[i] - 21] + '0';
    else if (input[i] == 32)
      output = output + morseTable[0];
  }
  return output;
} // End of Function "formMorse"
