/********************************************************************************
 * TE237 Sensores e Instrumentação 
 * Professor: Dr. José Carlos da Cunha
 * TRABALHO DE CONCLUSÃO DE DISCIPLINA
 *  Título: MONITOR WIFI DE FREQUÊNCIA CARDÍACA  
 ********************************************************************************/

/* Parâmetros e credenciais da rede WiFi utilizada*/
#include <WiFi.h>
const char* ssid     = "tcd";
const char* password = "12345678";
WiFiServer server(80);

//Variáveis Globais
float frq;
int value = 0;

//Analog Input: Pino de leitura analógica do sinal
#define ANALOG_PIN_0 34

void setup()
{
  Serial.begin(115200);
  delay(10);

  connectWiFi();
}

void loop()
{
  WiFiLocalWebPageCtrl();
}

//Realiza a leitura analógica do pino 34 e realiza o cálculo da frequência cardíaca em BPM
void getFeqC()
{
  unsigned long t = millis();
  unsigned long tMax = millis();
  int pulsos = 0;
  int pico = 0;
  int Periodo = 0;
  int mPeriodos = 0;
  int value = 0;

  //Leitura realizada durante 4 pulsos
  while(pulsos < 4){
    delay(10);
    value = analogRead(ANALOG_PIN_0);

    //Tempo limite de leitura
    if((millis() - tMax) > 8000){
      break;
      }

    if(value > 3600 && pico == 0){
        pulsos ++;
        if(pulsos>0){
          Periodo = millis() - t;
          if(Periodo < 350){
              pulsos--;
          }
          else if(Periodo > 1500){
              pulsos--;
              if(Periodo > 2000){
                pulsos = 0;
                mPeriodos = 0;
              }
          }
          else{
            mPeriodos = mPeriodos+Periodo;
          }
          
        }
        pico = 1; 
        t = millis();
      }
    if(value < 500 && pico == 1){
        pico = 0;
    }
  }
  mPeriodos = mPeriodos/(pulsos-1);
  frq = 1.0/(mPeriodos/1000.0)*60;
}

void WiFiLocalWebPageCtrl(void)
{
  WiFiClient client = server.available();   // Aguarda por um web client
  if (client) {                             // Se um Web Client está conectado
    getFeqC();                              // Chama função que lê e calcula a frequência cardíaca
    Serial.println("New Client.");          
    String currentLine = "";                
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
            
                                            // if the current line is blank, you got two newline characters in a row.
                                            // that's the end of the client HTTP request, so send a response with the BPM frequency value:
          if (currentLine.length() == 0) {
            String sHTML;
            sHTML  = "<!doctype html>";
            sHTML += "<html>";
            sHTML += "<head>";
            sHTML += "<meta http-equiv=\"refresh\" content=\"1\">";
            sHTML += "</head>";
            sHTML += "<body style=\"background-color:#7997A1\">"; 
            sHTML += "<div style=\"position: relative; width: 100%; height: 40rem;\">";
            sHTML += "<div style=\"position: absolute; left: 0;top: 35%;width: 100%;text-align: center;font-size: 18px;\">";          
            sHTML += "<h1 style=\"font-size:40px; color: white\">Frequencia cardiaca</h1><h2 style=\"font-size:50px; color: white\">";
            sHTML += frq;
            sHTML += " BPM</h2></div></div>";
            sHTML += "<br>";
            sHTML += "</body>";
            sHTML += "</html>"; 
            client.print(sHTML);
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

//Connecting to a WiFi network
void connectWiFi(void)
{
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}
