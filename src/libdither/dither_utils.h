#pragma once
#ifndef DITHER_UTILS_H
#define DITHER_UTILS_H

#include <stdint.h>
#include <stdbool.h>

/* Helper function to check if a pixel should be processed based on dot_size and dot_spacing */
static inline bool should_process_pixel(int x, int y, int dot_size, int dot_spacing) {
    if (dot_size < 1) dot_size = 1;
    if (dot_spacing < 0) dot_spacing = 0;
    int step = dot_size + dot_spacing;
    return (x % step == 0) && (y % step == 0);
}

/* Helper function to set a pixel with dot_size applied */
static inline void set_pixel_with_dot_size(uint8_t* out, int img_width, int img_height, 
                                            int x, int y, uint8_t value, int dot_size) {
    if (dot_size < 1) dot_size = 1;
    
    for(int dy = 0; dy < dot_size; dy++) {
        for(int dx = 0; dx < dot_size; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                out[py * img_width + px] = value;
            }
        }
    }
}

/* Helper function to set a color pixel with dot_size applied */
static inline void set_color_pixel_with_dot_size(int* out, int img_width, int img_height,
                                                   int x, int y, int value, int dot_size) {
    if (dot_size < 1) dot_size = 1;
    
    for(int dy = 0; dy < dot_size; dy++) {
        for(int dx = 0; dx < dot_size; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                out[py * img_width + px] = value;
            }
        }
    }
}

#endif /* DITHER_UTILS_H */
