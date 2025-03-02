/*

    qoi_viewer.h

    This header contains declaration of functions related to QOI viewer

    Code licensed under MIT License

    Copyright (c) 2025 Aftersol

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/
/// @file qoi_viewer.h
/// @brief This header contains declaration of functions related to QOI viewer

#ifndef QOI_VIEWER_H
#define QOI_VIEWER_H

#if __cplusplus
extern "C" {
#endif

#include "sQOI.h"
#include <stdint.h>
#include <stdbool.h>
#include <libdragon.h>

// Image buffer size: 320*240*4
#define IMG_BUFFER_SIZE 307200

extern uint8_t buffer0[IMG_BUFFER_SIZE];

extern uint8_t buffer1[IMG_BUFFER_SIZE];

typedef enum qoi_error_code {
    QOI_NOT_INITALIZED = -1,
    QOI_OK, 
    QOI_NULL_BUFFER,
    QOI_INVAILD_FILE,
    QOI_NO_FILE,
    QOI_NO_FILENAME
} qoi_error_code;

typedef struct qoi_img_info {
    /// @brief Width of the QOI image
    int width;

    /// @brief Height of the QOI image
    int height;

    /// @brief Number of channels of the QOI image where 3 is RGB and 4 is RGBA
    int channels;

    /// @brief Error code as the result of decoding
    qoi_error_code error;

    /// @brief Decoding time in seconds
    float decodeTime;

    /// @brief Names of QOI file
    char name[256];

    /// @brief Whether to toggle displaying debug text upon pressing the Start button on the N64 controller
    bool renderDebugFont;
} qoi_img_info_t;

void draw_image(surface_t* disp, qoi_img_info_t info);
void openQOIFile(const char* filename, uint8_t* bytes, qoi_img_info_t* info);

#if __cplusplus
}
#endif

#endif // QOI_VIEWER_H