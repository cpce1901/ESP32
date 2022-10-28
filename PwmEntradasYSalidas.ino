const int ledPin = 16;  // GPIO corresponde a PIN 16


const int freq = 10000;      //Se declara la frecuencia de trabajo ej: 1000 Hz
const int ledChannel = 0;   //Se declara el canal de trabajo - EL ESP tiene 16 canalaes diferentes
const int resolution = 8;   //Se declara la resolucion - Se puede escoger desde 1Bit (2^1 - 2 valores) hasta 16Bits (2^16 - 65536 valores)
 
void setup(){
  Serial.begin(115200);

  ledcSetup(ledChannel, freq, resolution);  // Se declara el objeto PWM y sus parametros
  
  ledcAttachPin(ledPin, ledChannel);    //Se declara el modo de trabajo del pin que se contempla colocar en PWM
}
 
void loop(){
  
  for(int i = 0; i <= 255; i++){   
    ledcWrite(ledChannel, i); //Se utiliza para realizar una accion en la salida
    delay(20);
    Serial.println(i);
   
  }

 
  for(int i = 255; i  >= 0; i--){
    ledcWrite(ledChannel, i);   
    delay(20);
    Serial.println(i);
  }

   
}
