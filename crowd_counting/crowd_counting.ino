/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Basado en ejemplo person_detection de libreria Arduino_TensorFlowLite
==============================================================================*/

// DEBUG false para probar sin PC
#define DEBUG true
#define PERSON_THRESHOLD 0.5
// SALAS_ID a probar
// 16: A-016,46: Testamento, 34: LDS
#define SALA_ID "016"
#define Serial2 SerialLoRa

#include "camera.h"
#include <SDRAM.h>

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "image_provider.h"
#include "main_functions.h"
#include "model_settings.h"
#include "cc_model_data.h"
#include "ImageCropper.h"
//#include "connected_component_count.h"
//#include "detection_responder.h"

/*
#include "sdcard.h"
#include "SDMMCBlockDevice.h"
#include "FATFileSystem.h"
*/


// Globals, used for compatibility with Arduino-style sketches.
namespace {
  
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  
  // In order to use optimized tensorflow lite kernels, a signed int8_t quantized
  // model is preferred over the legacy unsigned model format. This means that
  // throughout this project, input images must be converted from unisgned to
  // signed format. The easiest and quickest way to convert from unsigned to
  // signed 8-bit integers is to subtract 128 from the unsigned value to get a
  // signed value.
  
  // An area of memory to use for input, output, and intermediate arrays.
  constexpr int kTensorArenaSize = 3 * 1024 * 1024;
  // Keep aligned to 16 bytes for CMSIS
  //alignas(16) uint8_t tensor_arena[kTensorArenaSize];

  uint8_t* tensor_arena = (uint8_t*) SDRAM_START_ADDRESS; // se asigna primero la imagen, y luego tensor_arena
  //alignas(16) uint8_t *tensor_arena = (uint8_t *)SDRAM.malloc(kTensorArenaSize * sizeof(uint8_t));

/*
  // SD Card block device and filesystem
  SDMMCBlockDevice block_device;
  mbed::FATFileSystem fs("fs");
*/

  int it1,it2,it3; //iteradores
  int image_count = 0;
  constexpr int images_to_write = 3;

  constexpr int img_width = 320;
  constexpr int img_height = 240;

  // dimension de output de modelo de reconocimiento
  constexpr int output_dim = 30;

  // Buffer para imagenes capturadas
  FrameBuffer fbImage(img_width, img_height, 2);
  
  // Arreglo de imagen recortada
  uint8_t croppedImage[kMaxImageSize];
//  uint8_t* croppedImage = (uint8_t*) SDRAM_START_ADDRESS;
  //uint8_t* croppedImage = (uint8_t *)SDRAM.malloc(kMaxImageSize * sizeof(uint8_t));
  ImageCropper image_cropper;

  // mapa de segmentacion binario
  uint8_t **bin_segmentation_map;

  // Resultado de Crowd Counting
  uint8_t crowd_count;

  // parametros envio por LoRa
  const int messageLen = 16;
  char crowd_count_str[4] = "";
  char mensajeLoRa[messageLen] = "";
  char begin_fmt[6] = "<stL>";
  char end_fmt[5] = "<eL>";

  //boolean sd_card_initialized = false;
  
  //char filename[255];
  
}  // namespace


// Indicacion visual al capturar imagenes
void countDownBlink(){
    for (int i = 0; i < 6; i++){
        digitalWrite(LEDG, i % 2);
        delay(500);
    }
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, LOW);
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

  // inicializar SDRAM
  SDRAM.begin(SDRAM_START_ADDRESS + kTensorArenaSize + 2 * 1024);
  
  tflite::InitializeTarget();
/*
  // inicializar mapa de segmentacion binario
  bin_segmentation_map = new uint8_t*[output_dim];
  for (it1 = 0; it1 < output_dim; it1++){
    bin_segmentation_map[it1] = new uint8_t[output_dim];
  }
  */
/*
  // inicializar SD
  Serial.println("Mounting SD Card...");
  sd_card_initialized = mountSDCard(&fs, &block_device);
  if (sd_card_initialized){
    Serial.println("SD Card mounted.");
  }
*/

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(cc_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    if (DEBUG){
      Serial.print(
          "Model provided is schema version %d not equal "
          "to supported version ");
      Serial.println(model->version(), TFLITE_SCHEMA_VERSION);
    }
    return;
  }

  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<9> micro_op_resolver;
  micro_op_resolver.AddAdd();
  micro_op_resolver.AddConcatenation();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddPad();
  micro_op_resolver.AddRelu();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();
  
  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(model, 
                                                    micro_op_resolver, 
                                                    tensor_arena, 
                                                    kTensorArenaSize);
  
  interpreter = &static_interpreter;
  
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    if (DEBUG){
      Serial.println("AllocateTensors() failed");
    }
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);

  if ((input->dims->size != 4) || (input->dims->data[0] != 1) ||
      (input->dims->data[1] != kNumRows) ||
      (input->dims->data[2] != kNumCols) ||
      (input->dims->data[3] != kNumChannels) || (input->type != kTfLiteInt8)) {
    if (DEBUG){
      Serial.println("Bad input tensor parameters in model");
    }
    return;
  }
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
  // asumiendo que fbImage esta llegando bien a cam.grabFrame, sigamos
  if (kTfLiteOk != GetImage(img_width, img_height, &fbImage) && DEBUG) {
    Serial.println("Image capture failed.");
  }
  digitalWrite(LEDB, HIGH);


// crop the image to the size that the model expects
  if (DEBUG){
    Serial.print("Recortando imagen a ");
    Serial.print(kNumCols);
    Serial.print("x");
    Serial.println(kNumRows);
  }
  /*int crop_result = */image_cropper.crop_image(fbImage.getBuffer(), img_width, img_height, croppedImage, kNumCols, kNumRows);
  /*if(crop_result < 0) {
    Serial.println("Failed to crop image");
    return;
  }*/

// igual para tener mas imagen, podria cortarla a 240x240, y despues transformarla a 224x224

  if (DEBUG){
    Serial.println("\n- - - - - - START INPUT - - - - - -\n");
    
    it3 = 0;
    Serial.print("[");
    for (it1 = 0; it1 < kNumRows; it1++){
      Serial.print("[");
      for (it2 = 0; it2 < kNumCols; it2++){
        Serial.print((String)croppedImage[it3] + ",");
        it3++;
      }
      Serial.println("],");
    }
    Serial.println("]");
    
    Serial.println("\n - - - - - - END INPUT - - - - - -\n");
  }


  /*
  // almacenar imagen en SD
  if(sd_card_initialized && image_count < images_to_write) {
    Serial.print("Writing cropped image ");
    Serial.print(image_count);
    Serial.println(" to SD card");
  
    //write_image_to_sd_card("cropped", croppedImage, kNumRows, kNumCols, kMaxImageSize, image_count);
    saveImage(fbImage.getBuffer(), filename, image_count);
  }
  else if (image_count == images_to_write){

    // intentar abrir uno de los archivos de la sd e imprimirlo
    
    fs.unmount();
    Serial.println("SD desmontada");  
  }
  */

  // copy the scaled image to the TF input
  if (DEBUG){
    Serial.println("Copying cropped image to TensorFlow model");
  }
  memcpy(input->data.uint8, croppedImage, kMaxImageSize);

  // Run the model on this input and make sure it succeeds.
  if (DEBUG){
    Serial.println("Invoking the TensorFlow interpreter");
  }

  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke() && DEBUG) {
    Serial.println("Invoke failed.");
  }

  TfLiteTensor* output = interpreter->output(0);
  
  // Dimensionalidad output: 1x30x30x2
  /*
  if (DEBUG){
    Serial.print("Dimensionalidad output: ");
    for (it1 = 0; it1 < output->dims->size; it1++){
      Serial.print(output->dims->data[it1]);
      Serial.print(" ");
    }
    Serial.println("");
  }
*/
/*
  if (DEBUG){
    Serial.println("\n- - - - - - START OUTPUT - - - - - -\n");
    
    uint8_t output_item;
    it3 = 0;
    Serial.print("CALMAO, el person_threshold es:");
    Serial.println(PERSON_THRESHOLD*255.0);
    Serial.print("[");
    for (it1 = 0; it1 < output_dim; it1++){
      Serial.print("[");
      for (it2 = 0; it2 < output_dim; it2++){
        
        output_item = output->data.uint8[it3*2];
        
        // quizas al obtenerlo en esta linea, se convierte en un arreglo 1-dimensional, desordenando todo ;-; // efectivamente
        Serial.print(output_item);
        Serial.print(",");
        
        if (output_item < (int8_t)(PERSON_THRESHOLD*255)){
          bin_segmentation_map[it1][it2] = (uint8_t)0;
        }
        else {
          bin_segmentation_map[it1][it2] = (uint8_t)1;
        }
        
        it3++;
      }
      Serial.println("],");
    }
    Serial.println("]");
    
    Serial.println("= = = = = = = = = = = = = = = = = = = ==  == =\n");
    
    // Segundo mapa de segmentacion
    it3 = 0;
    Serial.print("[");
    for (it1 = 0; it1 < output_dim; it1++){
      Serial.print("[");
      for (it2 = 0; it2 < output_dim; it2++){
        
        Serial.print(output->data.uint8[it3*2+1]);
        Serial.print(",");
        
        it3++;
      }
      Serial.println("],");
    }
    Serial.print("]");
    
    
    Serial.println("\n - - - - - - END OUTPUT - - - - - -\n");
    
    
    Serial.println("\n- - - - - - START BIN_SEG_MAP - - - - - -\n");
    
    Serial.print("[");
    for (it1 = 0; it1 < output_dim; it1++){
      Serial.print("[");
      for (it2 = 0; it2 < output_dim; it2++){
        
        Serial.print(bin_segmentation_map[it1][it2]);
        Serial.print(",");
      }
      Serial.println("],");
    }
    Serial.print("]");
    
    Serial.println("- - - - - - END BIN_SEG_MAP - - - - - -\n");
  }
  
  // Fallo aqui en la foto num 55-56??
  
    crowd_count = find_components(output_dim, bin_segmentation_map);
  */
  /*
  if (DEBUG){
    Serial.print("[");
    for (it2 = 0; it2 < 100; it2++){
      Serial.print(output->data.int8[it2]);
      Serial.print(",");
    }
    Serial.println("],");
  }
*/
  /*
  crowd_count = output->data.uint8;
  if (DEBUG){
    Serial.print("\n* * * Crowd count: ");
    Serial.print(crowd_count);
    Serial.println("* * * * * * * * * * *\n");
  }
  */

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
  
  //char mySendArray[messageLen];
  //mensajeLoRa.toCharArray(mySendArray, messageLen);

  SerialLoRa.write(mensajeLoRa, messageLen);
  if (DEBUG){
    Serial.print("Mensaje enviado: ");
    Serial.println(mensajeLoRa);

    Serial.println("loop -");
  }
  
/*
  // Process the inference results.
  int8_t person_score = outp t->data.uint8[k BackgroundIndex];
  int8_t no_person_score = output->data.uint8[kPersonIndex];
  float person_score_f =
      (person_score - output->params.zero_point) * output->params.scale;
  float no_person_score_f =
      (no_person_score - output->params.zero_point) * output->params.scale;
  RespondToDetection(person_score_f, no_person_score_f);
  */
  
  image_count++;
  delay(10000);
}
