/*

    qoi_viewer.c

    This source code implements functions related to QOI viewer

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

/// @file qoi_viewer.c
/// @brief This source code implements functions related to QOI viewer

#define SIMPLIFIED_QOI_IMPLEMENTATION

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "sQOI.h"
#include "qoi_viewer.h"

#include <assert.h>

/// @brief This function draws image decoded from QOI
/// @param disp Surface image
/// @param info QOI info for drawing image properly
void draw_image(surface_t* disp, qoi_img_info_t info) {
    const char rgbStr[] = "RGB";
    const char rgbaStr[] = "RGBA";
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

    if (info.renderDebugFont == true) {
        if (info.channels == 3) {
            channelStr = rgbStr;
        } else if (info.channels == 4) {
            channelStr = rgbaStr;
        }
        else {
            channelStr = unknownStr;
        }
    
        rdpq_text_printf(
            &(rdpq_textparms_t) {
                .width = 320-32,
                .align = ALIGN_LEFT,
                .wrap = WRAP_WORD,
            }, 
            1, 
            32, 
            32, 
            "Current Image: %s\nSize: %i x %i\nChannels: %i (%s)\nDecode Time: %f ms",
            info.name,
            info.width,
            info.height,
            info.channels,
            channelStr,
            info.decodeTime * 1000.0f
            );
    }

    rdpq_detach_show();
}


/// @brief This function decodes QOI file from from into the framebuffer
/// @param filename Name of the QOI file
/// @param bytes Pointer to a raw image buffer
/// @param info QOI decoding info as a result of decoding qoi file
void openQOIFile(const char* filename, uint8_t* bytes, qoi_img_info_t* info) {
    
    qoi_desc_t desc;
    qoi_dec_t dec;
    qoi_pixel_t px;
    uint8_t* qoi_bytes;
    int seek = 0, buffer_size;
    long long start, end;

    FILE* fp;

    if (!bytes) {
        info->error = QOI_NULL_BUFFER;
        return;
    }

    if (!filename) {
        info->error = QOI_NO_FILENAME;
        return;
    }

    start = timer_ticks();
    fp = fopen(filename, "rb");

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

    assertf(
        info->width * info->height * info->channels <= IMG_BUFFER_SIZE,
        "%s is too big to open and read\n\
        To prevent buffer overrun on big sized QOI images,\n\
         QOI Viewer has been terminated.\n\
        Make sure your QOI image is a maximum of 320 pixels\n\
         in width and 240 pixels in height",
        filename
    ); // crash to prevent buffer overrun

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
        
        // O2 still copy bytes so we use pointer trick to pretend
        // that pixel data is an integer and copy 4 bytes at a time
        *(uint32_t*)(bytes+seek) = px.concatenated_pixel_values;
        
        seek += 4;

    }
    
    memset(info->name, 0, 256);

    // copy first 255 characters to prevent string overflow
    memcpy(info->name, filename, strlen(filename) < 256 ? strlen(filename) : 255);
    info->error = QOI_OK;

cleanup:
    free(qoi_bytes);
    qoi_bytes = NULL;
    end = timer_ticks();
    info->decodeTime = (float)((float)(end - start) / (float)TICKS_PER_SECOND);
}