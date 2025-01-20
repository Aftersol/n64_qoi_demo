/*

    main.c

    This file is the entry point of the N64 QOI Viewer ROM

    TODO: Refactor main.c into functions and files

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

#include <stdio.h>
#include <string.h>

#include <libdragon.h>
#include "../libdragon/include/joypad.h"
#include "../libdragon/include/joybus.h"

#include "qoi_viewer.h"

uint8_t buffer0[IMG_BUFFER_SIZE];
uint8_t buffer1[IMG_BUFFER_SIZE];

// files to load from filesystem
char* names[] = {
    "rom:/overscan.qoi",
    "rom:/smpte_color_bars.qoi",
    "rom:/ebu_colour_bars.qoi",
    "rom:/pal_pm5544.qoi",
    "rom:/dice.qoi",
    "rom:/edgecase.qoi",
    "rom:/kodim10.qoi",
    "rom:/kodim23.qoi",
    "rom:/qoi_logo.qoi",
    "rom:/testcard_rgba.qoi",
    "rom:/testcard.qoi",
    "rom:/wikipedia_008.qoi",
    "rom:/qrcode.qoi",
    "rom:/credits.qoi"
};

int main(void)
{
    long long start, end;
    int index = 0, prev_index = 0;
    int name_arr_size = 14; // change this line to match the amount of images
    
    qoi_img_info_t info = (qoi_img_info_t) {
        .width = 0,
        .height = 0,
        .channels = 0,
        .error = QOI_OK
    };

    rdpq_font_t *font;

    console_init();

    debug_init_usblog();
    console_set_debug(true);

    timer_init();
    joypad_init();

    dfs_init(DFS_DEFAULT_LOCATION);
    
    memset(buffer0, 0, IMG_BUFFER_SIZE); // clear the buffer

    start = timer_ticks();
    openQOIFile(names[0], &buffer0[0], &info);
    end = timer_ticks();


    // somehow double buffering
    // the image 
    // fixes black lines at the
    // bottom of the screen
    memcpy(buffer1, buffer0, IMG_BUFFER_SIZE);  
    
    assert(info.error == QOI_OK);

    printf(
        "decoded %s in %f ms!\n",
        names[0],
        ((float)end - (float)start) / 1000.0f
    ); // time in ms spent decoding
    
    printf(
        "First pixel of %s: %i %i %i %i\n", 
        names[0],
        buffer0[0],
        buffer0[1],
        buffer0[2],
        buffer0[3]
    ); // get color of first pixel
    
    wait_ms(1000);

    console_clear();
    console_close();

    timer_close();

    // QOI only supports 32 bit RGBA image
    // so set display bits to 32 bits per pixel
    display_init(
        RESOLUTION_320x240,
        DEPTH_32_BPP,
        2,
        GAMMA_NONE,
        FILTERS_RESAMPLE
    );

    rdpq_init();
    rdpq_set_mode_standard();

    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, font);

    while(1) {
        surface_t* disp;

        joypad_poll();

        // read first controller port
        joypad_port_t port = JOYPAD_PORT_1;
        joypad_inputs_t input = joypad_get_inputs(port); 

        while(!(disp = display_try_get())) {;}

        // go to previous image if left is pressed
        if (
            input.btn.b || 
            input.btn.d_left || 
            input.btn.l || 
            input.btn.c_left ||
            joypad_get_axis_pressed(port, JOYPAD_AXIS_STICK_X) == -1
        ) {
            index--;
            if (index == -1) {
                index = name_arr_size - 1;
            }

        }

        // advance to next image if right is pressed
        if (
            input.btn.a || 
            input.btn.d_right || 
            input.btn.r || 
            input.btn.c_right ||
            joypad_get_axis_pressed(port, JOYPAD_AXIS_STICK_X) == 1
        ) {
            index++;
            if (index == name_arr_size) {
                index = 0;
            }
        }

        // load next image upon pressing left or right
        if (prev_index != index) {
            prev_index = index;

            openQOIFile(names[index], buffer0, &info);
            memcpy(buffer1, buffer0, IMG_BUFFER_SIZE);

            assert(!info.error);
        }

        rdpq_attach(disp, NULL);

        surface_t image = surface_make_linear(
            buffer0,
            FMT_RGBA32,
            info.width,
            info.height
        );

        rdpq_set_fill_color(RGBA32(0, 0, 0, 255));
        rdpq_fill_rectangle(0, 0, 320, 240);
        
        // draw decoded image into screen
        rdpq_tex_blit(&image, 0.0, 0.0, NULL);


        // this causes framebuffer to turn black and white 
        // with distored image so commented out
        /*rdpq_text_printf(
            NULL, 
            1, 
            32, 
            32, 
            "Current Image: %s\nSize: %i x %i\nChannels: %i",
            names[index],
            info.width,
            info.height,
            info.channels
            );*/ 


        rdpq_detach_show();
        
    }
}