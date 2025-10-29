#define MODULE_API_EXPORTS
#include <stdlib.h>
#include "libdither.h"
#include "dither_utils.h"
#include "random.h"

MODULE_API double auto_threshold(const DitherImage* img) {
    /* automatically determines the best threshold value for the image.
     * use output of this function as threshold parameter for the threshold dither function */
    double avg = 0.0;
    double min = 1.0;
    double max = 0.0;
    size_t imgsize = (size_t)(img -> width * img -> height);
    for(size_t i = 0; i < imgsize; i++) {
        double c = gamma_encode(img -> buffer[i]);
        avg += c;
        if(c < min) min = c;
        if(c > max) max = c;
    }
    avg /= (double)(img -> width * img -> height);
    double v = (1.0 - (max - min)) * 0.5;
    if(avg < gamma_decode(0.5))
        v = -v;
    return gamma_decode(avg + v);
}

MODULE_API void threshold_dither(const DitherImage* img, double threshold, double noise, int dot_size, int dot_spacing, uint8_t* out) {
    /* Threshold dithering
     * threshold: threshold to dither a pixel black. From 0.0 to 1.0. Suggested value: 0.5.
     * noise: amount of noise / randomness in pixel placement
     * */
    if (dot_size < 1) dot_size = 1;
    if (dot_spacing < 0) dot_spacing = 0;
    
    size_t addr = 0;
    threshold = (0.5 * noise + threshold * (1.0 - noise));
    for(int y = 0; y < img -> height; y++) {
        for(int x = 0; x < img -> width; x++) {
            if (img->transparency[addr] != 0) {
                double px = img->buffer[addr];
                if (noise > 0)
                    px += (rand_float() - 0.5) * noise;
                if (px > threshold && should_process_pixel(x, y, dot_size, dot_spacing))
                    set_pixel_with_dot_size(out, img->width, img->height, x, y, 0xff, dot_size);
            } else
                out[addr] = 128;
            addr++;
        }
    }
}
