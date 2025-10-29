#define MODULE_API_EXPORTS
#include <stdlib.h>
#include <string.h>
#include "libdither.h"
#include "dither_utils.h"
#include "dither_dotlippens_data.h"

MODULE_API int* create_dot_lippens_cm(void) {
    int cm[4][16][16];
    for(size_t i = 0; i < 16; i++) {
        for(size_t j = 0; j < 16; j++) {
            cm[3][i][j] = ocm[i][j];
            cm[2][i][j] = ocm[15 - i][15 - j];
            cm[1][i][j] = ocm[15 - j][15 - i];
            cm[0][i][j] = ocm[j][i];
        }
    }
    int* final_cm = (int*)calloc(128 * 128, sizeof(int));
    for(size_t i = 0; i < 128; i += 16)
        for(size_t j = 0; j < 128; j += 16)
            for(size_t m = 0; m < 16; m++)
                for(size_t n = 0; n < 16; n++)
                    final_cm[(i + m) * 128 + (j + n)] = cm[order[(int)((float)i / 16.0)][(int)((float)j / 16.0)]][m][n];
    return final_cm;
}

MODULE_API DotLippensCoefficients* DotLippensCoefficients_new(int width, int height, const int* coefficients) {
    DotLippensCoefficients* self = calloc(1, sizeof(DotLippensCoefficients));
    size_t size = (size_t)(width * height);
    self->buffer = (int*)calloc(size, sizeof(int));
    memcpy(self->buffer, coefficients, size * sizeof(int));
    self->height = height;
    self->width = width;
    return self;
}

MODULE_API void DotLippensCoefficients_free(DotLippensCoefficients* self) {
    if(self) {
        free(self->buffer);
        free(self);
        self = NULL;
    }
}

MODULE_API DotClassMatrix* get_dotlippens_class_matrix(void) { return DotClassMatrix_new(128, 128, dotlippens_class_matrix); }
MODULE_API DotLippensCoefficients* get_dotlippens_coefficients1(void) { return DotLippensCoefficients_new(5, 5, dotlippens1_coe); }
MODULE_API DotLippensCoefficients* get_dotlippens_coefficients2(void) { return DotLippensCoefficients_new(5, 5, dotlippens2_coe); }
MODULE_API DotLippensCoefficients* get_dotlippens_coefficients3(void) { return DotLippensCoefficients_new(5, 5, dotlippens3_coe); }

MODULE_API void dotlippens_dither(const DitherImage* img, const DotClassMatrix* class_matrix, const DotLippensCoefficients* coefficients, int dot_size, int dot_spacing, uint8_t* out) {
    /* Lippens and Philips Dot Dithering
     * class_matix: same class matrix as used by regular (Knuth's) dot ditherer
     * coefficients: Lippens and Philips coefficients */
    (void)dot_size;
    (void)dot_spacing;
    double coefficients_sum = 0.0;
    for(int i = 0; i < coefficients->width * coefficients->height; i++)
        coefficients_sum += (double)coefficients->buffer[i];
    coefficients_sum /= 2.0;

    size_t image_size = (size_t)(img->width * img->height);
    int* image_cm = (int*)calloc(image_size, sizeof(int));
    double* image = (double*)calloc(image_size, sizeof(double));

    for(int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            size_t addr = (size_t)(y * img->width + x);
            image_cm[addr] = class_matrix->buffer[(y % class_matrix->height) * class_matrix->width + (x % class_matrix->width)];
            image[addr] = img->buffer[addr];  // make a copy of the image as we can't modify the original
        }
    }
    int half_size = (int)(((float)coefficients->width - 1.0) / 2.0);
    int n = 0;
    while(n != 256) {
        for(int y = 0; y < img->height; y++) {
            for (int x = 0; x < img->width; x++) {
                size_t addr = (size_t)(y * img->width + x);
                if(image_cm[addr] == n) {
                    if (img->transparency[addr] != 0) {
                        double err = image[addr];
                        if (err > 0.5) {
                            err -= 1.0;
                            out[addr] = 0xff;
                        }
                        for (int cmy = -half_size; cmy <= half_size; cmy++) {
                            for (int cmx = -half_size; cmx <= half_size; cmx++) {
                                int imy = y + cmy;
                                int imx = x + cmx;
                                addr = (size_t)(imy * img->width + imx);
                                if (imy >= 0 && imy < img->height && imx >= 0 && imx < img->width)
                                    if (image_cm[addr] > cmx)
                                        image[addr] += err * (double) coefficients->buffer[
                                                (cmy + half_size) * coefficients->width + (cmx + half_size)] /
                                                       coefficients_sum;
                            }
                        }
                    } else
                        out[addr] = 128;
                }
            }
        }
        n++;
    }
    free(image_cm);
    free(image);
}
