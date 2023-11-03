
// DEBUG false para probar sin PC
#define DEBUG false
#define Serial2 SerialLoRa

/*
#include "sdcard.h"
#include "SDMMCBlockDevice.h"
#include "FATFileSystem.h"
*/


// Globals, used for compatibility with Arduino-style sketches.
namespace {
/*
  // SD Card block device and filesystem
  SDMMCBlockDevice block_device;
  mbed::FATFileSystem fs("fs");
*/

  int image_count = 0;
  constexpr int images_to_write = 100;

  // Resultado de Crowd Counting
  uint8_t crowd_count;

  // parametros envio por LoRa
  const int messageLen = 16;
  /*
  char crowd_count_str[4] = "";
  char mensajeLoRa[messageLen] = "";
  char begin_fmt[6] = "<stL>";
  char end_fmt[5] = "<eL>";
*/
  boolean sd_card_initialized = false;
  
  char filename[255];
  
}  // namespace


// Indicacion visual al capturar imagenes
void countDownBlink(){
    for (int i = 0; i < 6; i++){
        digitalWrite(LEDG, i % 2);
        delay(500);
    }
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, LOW);
    delay(500);
    digitalWrite(LEDB, HIGH);
    //digitalWrite(LEDG, LOW);
}


void setup() {

  // LoRa
  SerialLoRa.begin(9600); // for LoRa must be 9600
  while(!SerialLoRa){};
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  // Serial predeterminado
  Serial.begin(115200);
  if (DEBUG) {
    while (!Serial) {};
  }

/*
  // inicializar SD
  Serial.println("Mounting SD Card...");
  sd_card_initialized = mountSDCard(&fs, &block_device);
  if (sd_card_initialized){
    Serial.println("SD Card mounted.");
  }
*/
}


void loop() {

  if (DEBUG){
    Serial.println("==============================");
    Serial.println();
    Serial.print("Imagen nro. ");
    Serial.println(image_count);
  }


  // Get image from provider.
  countDownBlink();

/*
  crowd_count = image_count;
  
  sprintf(crowd_count_str, "%03d", crowd_count);

  if (DEBUG){
    Serial.print("Crowd count formateado: ");
    Serial.println(crowd_count_str);
  }
  
  // concatenar datos segun formato de envio
  strcpy(mensajeLoRa, begin_fmt);
  strcat(mensajeLoRa, SALA_ID);
  strcat(mensajeLoRa, crowd_count_str);
  strcat(mensajeLoRa, end_fmt);
  */
  //char mySendArray[messageLen];
  //mensajeLoRa.toCharArray(mySendArray, messageLen);

  SerialLoRa.write("<stL>testLR<eL>", messageLen);
  if (DEBUG){
    Serial.println("Mensaje enviado.");
    //Serial.println(mensajeLoRa);

    Serial.println("loop -");
  }
  
  image_count++;
  delay(10000);
}
