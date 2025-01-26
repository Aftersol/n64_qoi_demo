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

#define POOL_IMG_SIZE 15
#define MAX_STRING_SIZE MAX_FILENAME_LEN + 1
uint8_t buffer0[IMG_BUFFER_SIZE];
uint8_t buffer1[IMG_BUFFER_SIZE];

joypad_inputs_t joypad_poll_port(joypad_port_t port) {
    joypad_poll();
    return joypad_get_inputs(port); 
}

typedef struct name_node_pool_t name_node_pool_t;
struct name_node_pool_t {
    name_node_pool_t* prev;
    name_node_pool_t* next;
    
    int num_images;

    char name[POOL_IMG_SIZE][MAX_STRING_SIZE];
};

int main(void) {
    long long start, end;

    int index = 0, prev_index = 0;

    char sbuf[MAX_STRING_SIZE];
    
    name_node_pool_t start_node = (name_node_pool_t) {
        // loop back into itself if there is only one pool sector
        // upon user trying to enter the previous or next node
        // after user reaches the beginning or end of a node respectively
        .prev = &start_node, 
        .next = &start_node,
        .num_images = 0
    };
    name_node_pool_t* current_node = &start_node;
    
    qoi_img_info_t info = (qoi_img_info_t) {
        .width = 0,
        .height = 0,
        .channels = 0,
        .error = QOI_NOT_INITALIZED
    };

    rdpq_font_t *font;

    console_init();

    debug_init_usblog();
    console_set_debug(true);

    timer_init();
    joypad_init();

    dfs_init(DFS_DEFAULT_LOCATION);
    
    memset(sbuf, 0, MAX_STRING_SIZE);

    strncpy(sbuf, "rom:/", 6);
    if (dfs_dir_findfirst(".", sbuf+5) == FLAGS_FILE) {
        name_node_pool_t* node = &start_node;
        
        do {
            if (node->num_images >= POOL_IMG_SIZE) {
                // the program runs forever so no need to free pool
                name_node_pool_t* new_node = (name_node_pool_t*)malloc(sizeof(name_node_pool_t));
                
                assert(new_node != NULL);

                new_node->prev = node;
                new_node->next = &start_node;
                new_node->num_images = 0; 

                node->next = (name_node_pool_t*)new_node;
                start_node.prev = (name_node_pool_t*)new_node;

                node = (name_node_pool_t*)new_node;
            }

            memset(node->name[node->num_images], 0, MAX_STRING_SIZE);
            snprintf(node->name[node->num_images], MAX_STRING_SIZE - 1, sbuf);

            node->num_images++;
            
        } while (dfs_dir_findnext(sbuf+5) == FLAGS_FILE);
    }
    else { // you can't compile with an empty directory
        assert("No files found in ROM.");
    }

    memset(buffer0, 0, IMG_BUFFER_SIZE); // clear the buffer

    start = timer_ticks();
    openQOIFile(start_node.name[0], &buffer0[0], &info);
    end = timer_ticks();

    assert(info.error == QOI_OK);

    // somehow double buffering
    // the image 
    // fixes black lines at the
    // bottom of the screen
    memcpy(buffer1, buffer0, IMG_BUFFER_SIZE);  

    printf(
        "decoded %s in %f ms!\n",
        start_node.name[0],
        ((float)end - (float)start) / 1000.0f
    ); // time in ms spent decoding
    
    printf(
        "First pixel of %s: %i %i %i %i\n", 
        start_node.name[0],
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

    while (1) {
        surface_t* disp;

        while(!(disp = display_try_get())) {;}
        joypad_port_t port = JOYPAD_PORT_1;
        joypad_inputs_t input = joypad_poll_port(port);
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
                current_node = (name_node_pool_t*)current_node->prev;
                index = current_node->num_images - 1;
                assert(index >= 0);
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
            if (index >= current_node->num_images) {
                current_node = (name_node_pool_t*)current_node->next;
                index = 0;
            }
        }

        // load next image upon pressing left or right
        if (prev_index != index) {
            prev_index = index;

            //openQOIFile(names[index], buffer0, &info);

            openQOIFile(current_node->name[index], buffer0, &info);
            memcpy(buffer1, buffer0, IMG_BUFFER_SIZE);

            assert(info.error == QOI_OK);
        }

        draw_image(disp, info);
        
    }
}