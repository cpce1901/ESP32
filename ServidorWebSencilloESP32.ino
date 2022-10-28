#include <WiFi.h>
#define LED 2

String p = "off";

const char *ssid = "";
const char *password = "";

WiFiServer server(80);

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.println("eSTAMOS TRABAJANDO PARA USTED");

  WiFi.begin(ssid, password);

  Serial.println("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado la direccion IP es: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
 WiFiClient client = server.available();   

  if (client) {                             
    Serial.println(" Tenemos un nuevo cliente.");       
    String ultimaLinea = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char r = client.read();             
        Serial.write(r);                   
        
        if (r == '\n') {                    

          if (ultimaLinea.length() == 0) {    
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head> <title>ESP8266</title>");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.println("<style>html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button {border: none;color: white;padding: 15px 32px; text-align: center;");
            client.println("text-decoration: none;display: inline-block;font-size: 16px; margin: 4px 2px;cursor: pointer;}");
            client.println(".button1 {background-color: #008CBA;} /* AZUL */");
            client.println(".button2 {background-color: #4CAF50;} /* VERDE */");
            client.println("</style></head>");
            
            // Creación de botones 
            client.print("<body><h1>Prueba Servidor WEB ESP32</h1>");
            client.print("<body><h2>Comienzos en IOT con ESP32</h2>");

            if(p=="off"){
              client.println("<button type='button' class='button button1' onClick=location.href='/LED=ON'> PRENDE </button><br><br>");                          
                }
            else{
              client.println("<button type='button' class='button button2' onClick=location.href='/LED=OFF'> APAGA </button><br><br>");                                               
              }
         
            client.print("</body></html>");
            client.println();
            
            
            break;
          } else {    
            ultimaLinea = "";
          }
        } else if (r != '\r') {  
          ultimaLinea += r;      
        }

        
        if (ultimaLinea.indexOf("GET /LED=ON") != -1) {
          digitalWrite(LED, HIGH);               
          p="on";
        }
        if (ultimaLinea.indexOf("GET /LED=OFF") != -1) {
          digitalWrite(LED, LOW);                
          p="off";
        }
      }
    }
    
    client.stop();
    Serial.println("Cliente Desconectado");
  }
}
