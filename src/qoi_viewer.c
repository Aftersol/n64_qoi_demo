/*

    qoi_viewer.c

    This header contains implementation of functions related to QOI viewer

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


#define SIMPLIFIED_QOI_IMPLEMENTATION

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "sQOI.h"
#include "qoi_viewer.h"

#include <assert.h>

/// @brief This draw image decoded from QOI
/// @param disp Surface image
/// @param info QOI info for drawing image properly
inline void draw_image(surface_t* disp, qoi_img_info_t info) {
    const char rgbStr[] = "RGB";
    const char rgbaStr[] = "RGA";
    const char unknownStr[] = "???";
    const char* channelStr;
    
    surface_t image = surface_make_linear(
        buffer0,
        FMT_RGBA32,
        info.width,
        info.height
    );

    rdpq_attach(disp, NULL);

    rdpq_set_mode_standard();

    rdpq_set_fill_color(RGBA32(0, 0, 0, 255));
    rdpq_fill_rectangle(0, 0, 320, 240);

    // draw decoded image into screen
    rdpq_tex_blit(&image, 0.0, 0.0, NULL);

    if (info.channels == 3) {
        channelStr = rgbStr;
    } else if (info.channels == 4) {
        channelStr = rgbaStr;
    }
    else {
        channelStr = unknownStr;
    }

    rdpq_text_printf(
        NULL, 
        1, 
        32, 
        32, 
        "Current Image: %s\nSize: %i x %i\nChannels: %i (%s)",
        info.name,
        info.width,
        info.height,
        info.channels,
        channelStr
        );


    rdpq_detach_show();
}


/// @brief This function decodes QOI file from from into the framebuffer
/// @param filename Name of the QOI file
/// @param bytes Pointer to a raw image buffer
/// @param info QOI decoding info as a result of decoding qoi file
void openQOIFile(const char* filename, uint8_t* bytes, qoi_img_info_t* info) {
    
    if (!bytes) {
        info->error = QOI_NULL_BUFFER;
        return;
    }

    if (!filename) {
        info->error = QOI_NO_FILENAME;
        return;
    }

    qoi_desc_t desc;
    qoi_dec_t dec;
    qoi_pixel_t px;
    uint8_t* qoi_bytes;
    int seek = 0, buffer_size;

    FILE* fp = fopen(filename, "rb");

    if (!fp) {
        info->error = QOI_NO_FILE;
        return;
    }

    fseek(fp, 0, SEEK_END);
    buffer_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    qoi_bytes = (uint8_t*)malloc(buffer_size * sizeof(uint8_t));
    
    assert(qoi_bytes); // crash if qoi_bytes fails to allocate

    fread(qoi_bytes, 1, buffer_size, fp);

    fclose(fp);

    fp = NULL;

    qoi_desc_init(&desc);
    
    if (!read_qoi_header(&desc, qoi_bytes)) {
        info->error = QOI_INVAILD_FILE;
        goto cleanup;
    }

    info->width = desc.width;
    info->height = desc.height;
    info->channels = desc.channels;

    dec = (qoi_dec_t){
        .run = 0,
        .pad = 0,
        .pixel_seek = 0,
        .img_area = desc.width * desc.height,
        .qoi_len = buffer_size,
        .data = qoi_bytes,
        .offset = qoi_bytes + 14

    }; // somehow this compiles

    for (uint8_t element = 0; element < 64; element++)
        qoi_initalize_pixel(&dec.buffer[element]);

    qoi_set_pixel_rgba(&dec.prev_pixel, 0, 0, 0, 255);

    while (!qoi_dec_done(&dec)) {
        px = qoi_decode_chunk(&dec);
        
        assert(seek < IMG_BUFFER_SIZE); // crash to prevent buffer overrun
        
        bytes[seek] = px.red;
        bytes[seek+1] = px.green;
        bytes[seek+2] = px.blue;
        bytes[seek+3] = px.alpha;
        
        seek += 4;

    }
    
    memset(info->name, 0, 256);
    memcpy(info->name, filename, strlen(filename) < 256 ? strlen(filename) : 255);
    info->error = QOI_OK;

cleanup:
    free(qoi_bytes);
    qoi_bytes = NULL;
    
}