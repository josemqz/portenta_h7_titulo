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
*/

#include <string.h>

#include "Arduino.h"        // required for use of camera.h in .cpp file
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
            Serial.println("GetImage - InitCamera failed");
            return init_status;
        }
        g_is_camera_initialized = true;
    }

    delay(50);

    Serial.println("Capturando imagen...");
    if(cam.grabFrame(*image_data) != 0) {
        Serial.println("GetImage - grab failed");
        return kTfLiteError;
    }

    return kTfLiteOk;
}
