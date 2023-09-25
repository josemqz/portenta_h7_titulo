/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================

#include <algorithm>
#include <type_traits>

#include "image_provider.h"
#include "model_settings.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "test_over_serial/test_over_serial.h"

using namespace test_over_serial;

#if defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)
#define ARDUINO_EXCLUDE_CODE
#endif  // defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)

#ifndef ARDUINO_EXCLUDE_CODE

#include "Arduino.h"

namespace {

constexpr size_t kQQVGA_width = 320;   // pixels
constexpr size_t kQQVGA_height = 240;  // pixels

uint8_t image_buffer[kQQVGA_height * kQQVGA_width];
constexpr size_t kImageBufferLength =
    std::extent<decltype(image_buffer)>::value;

// Get the camera module ready
TfLiteStatus InitCamera() {
  // This function kept for future implementation
  MicroPrintf("OV7675 not yet supported.  Blank image will be substituted.");
  return kTfLiteOk;
}

// Begin the capture and wait for it to finish
TfLiteStatus PerformCapture() {
  // This function kept for future implementation
  MicroPrintf("Starting capture");
  delay(50);
  MicroPrintf("Image captured");
  return kTfLiteOk;
}

// Read data from the camera module into a local buffer
TfLiteStatus ReadData() {
  // This function kept for future implementation
  // until OV7675 supported, just fill with zeros (black image)
  std::fill_n(image_buffer, kImageBufferLength, 0);
  return kTfLiteOk;
}

// Decode the image, crop it, and convert it to grayscale
TfLiteStatus CropAndQuantizeImage(size_t image_width, size_t image_height,
                                  const TfLiteTensor* tensor) {
  MicroPrintf("Cropping image and quantizing");

  // cropping parameters
  const size_t vert_top = (image_height - kNumRows) / 2;
  const size_t vert_bottom = vert_top + kNumRows - 1;
  const size_t horz_left = (image_width - kNumCols) / 2;
  const size_t horz_right = horz_left + kNumCols - 1;

  const uint8_t* p = image_buffer + (vert_top * image_width);
  p += horz_left;
  int8_t* image_data = tensor->data.int8;
  for (size_t line = vert_top; line <= vert_bottom; line++) {
    for (size_t row = horz_left; row <= horz_right; row++, p++) {
      *image_data++ = tflite::FloatToQuantizedType<int8_t>(
          p[0] / 255.0f, tensor->params.scale, tensor->params.zero_point);
    }
    // move to next line
    p += ((image_width - 1) - horz_right) + horz_left;
  }

  MicroPrintf("Image cropped and quantized");
  return kTfLiteOk;
}

// Get an image from the camera module
TfLiteStatus GetCameraImage(const TfLiteTensor* tensor) {
  static bool g_is_camera_initialized = false;
  if (!g_is_camera_initialized) {
    TfLiteStatus init_status = InitCamera();
    if (init_status != kTfLiteOk) {
      MicroPrintf("InitCamera failed");
      return init_status;
    }
    g_is_camera_initialized = true;
  }

  TfLiteStatus capture_status = PerformCapture();
  if (capture_status != kTfLiteOk) {
    MicroPrintf("PerformCapture failed");
    return capture_status;
  }

  TfLiteStatus read_data_status = ReadData();
  if (read_data_status != kTfLiteOk) {
    MicroPrintf("ReadData failed");
    return read_data_status;
  }

  TfLiteStatus decode_status =
      CropAndQuantizeImage(kQQVGA_width, kQQVGA_height, tensor);
  if (decode_status != kTfLiteOk) {
    MicroPrintf("CropAndQuantizeImage failed");
    return decode_status;
  }

  return kTfLiteOk;
}

TfLiteStatus GetTestImage(TestOverSerial& test, const TfLiteTensor* tensor) {
  volatile bool done = false;
  volatile bool aborted = false;
  volatile size_t image_width = 0, image_height = 0;

  InputHandler handler = [&aborted, &done, &image_width,
                          &image_height](const InputBuffer* const input) {
    if (0 == input->offset) {
      if ((kQQVGA_height * kQQVGA_width) == input->total) {
        image_width = kQQVGA_width;
        image_height = kQQVGA_height;
      } else if ((kNumCols * kNumRows) == input->total) {
        image_width = kNumCols;
        image_height = kNumRows;
      } else {
        // image dimensions are not supported, abort input processing
        aborted = true;
        return false;
      }
    }

    std::copy_n(input->data.uint8, input->length, &image_buffer[input->offset]);
    if (input->total == (input->offset + input->length)) {
      done = true;
    }
    return true;
  };

  while (!done) {
    test.ProcessInput(&handler);
    if (aborted) {
      MicroPrintf("Input processing aborted");
      return kTfLiteError;
    }
    // wait for a full image from serial port before processing
    if (done) {
      TfLiteStatus decode_status =
          CropAndQuantizeImage(image_width, image_height, tensor);
      if (decode_status != kTfLiteOk) {
        MicroPrintf("CropAndQuantizeImage failed");
        return decode_status;
      }
    }
  }

  return kTfLiteOk;
}

}  // namespace

TfLiteStatus GetImage(const TfLiteTensor* tensor) {
  TestOverSerial& test = TestOverSerial::Instance(kIMAGE_GRAYSCALE);
  if (!test.IsTestMode()) {
    // check serial port for test mode command
    test.ProcessInput(nullptr);
  }
  if (test.IsTestMode()) {
    return GetTestImage(test, tensor);
  } else {
    // get an image from the camera
    return GetCameraImage(tensor);
  }
  // NOTREACHED
}

#endif  // ARDUINO_EXCLUDE_CODE
*/

#include <string.h>

#include "Arduino.h"        // required for use of camera.h in .cpp file
//#include "camera.h"         // header for the CameraClass class
#include "himax.h"
#include "image_provider.h"

#define SUPPORTED_IMAGE_WIDTH 320
#define SUPPORTED_IMAGE_HEIGHT 240

// Camera  instance
HM01B0 himax;
Camera cam(himax);

unsigned long lastUpdate = 0;
static bool g_is_camera_initialized = false;

// Get the camera module ready
TfLiteStatus InitCamera(int image_width, int image_height) {

    /**
     * RAF: This is odd, because the CameraClass allows you to pass pretty much any value for IMAGE_WIDTH and IMAGE_HEIGHT,
     * but as currently implemented, only actually works with a single resolution.
     */
     if(image_width != SUPPORTED_IMAGE_WIDTH || image_height != SUPPORTED_IMAGE_HEIGHT) {
        Serial.println("arduino_image_provider InitCamera: resolution not supported.");
        return kTfLiteError;
    }

    // Initialize the camera for QVGA, 30FPS
    bool begin_status = cam.begin(CAMERA_R320x240, CAMERA_GRAYSCALE, 30);
    if (!begin_status) {
        //TF_LITE_REPORT_ERROR(error_reporter, "arduino_image_provider InitCamera: camera.begin() failed with status %d", begin_status);
        Serial.println("arduino_image_provider InitCamera: camera.begin() failed with status " + (String)begin_status);
        return kTfLiteError;
    }
    else {
      Serial.println("Camera successfully initialized.\n");  
    }

    return kTfLiteOk;
}


// Get an image from the camera module
TfLiteStatus GetImage(int image_width, int image_height, FrameBuffer* image_data) {

    if (!g_is_camera_initialized) {
         TfLiteStatus init_status = InitCamera(image_width, image_height);
        if (init_status != kTfLiteOk) {
            //TF_LITE_REPORT_ERROR(error_reporter, "GetImage - InitCamera failed");
            Serial.println("GetImage - InitCamera failed");
            return init_status;
        }
        g_is_camera_initialized = true;
    }

    delay(50);

    Serial.println("Capturando imagen...");
    if(cam.grabFrame(*image_data) != 0) {
        //TF_LITE_REPORT_ERROR(error_reporter, "GetImage - grab failed");
        Serial.println("GetImage - grab failed");
        return kTfLiteError;
    }

    return kTfLiteOk;
}