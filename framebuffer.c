#include "framebuffer.h"
#include "raytracing.h"
#include <stdio.h>
#include <stdint.h>

static unsigned char a_byte(double x) {
    if (x <= 0.0) return 0;
    if (x >= 1.0) return 255;
    return (unsigned char)(x * 255.0 + 0.5);
}

int guardar_avs(const char *ruta, const Escena *escena) {
    if (!ruta || !escena || escena->ancho <= 0 || escena->alto <= 0) return 0;
    FILE *f = fopen(ruta, "wb");
    if (!f) return 0;
    uint32_t w = (uint32_t)escena->ancho, h = (uint32_t)escena->alto;
    unsigned char cabecera[8] = {
        (unsigned char)(w >> 24), (unsigned char)(w >> 16), (unsigned char)(w >> 8), (unsigned char)w,
        (unsigned char)(h >> 24), (unsigned char)(h >> 16), (unsigned char)(h >> 8), (unsigned char)h
    };
    int ok = fwrite(cabecera, 1, 8, f) == 8;
    for (int y = 0; ok && y < escena->alto; y++) {
        for (int x = 0; x < escena->ancho; x++) {
            Color c = trazar_pixel(escena, x, y);
            unsigned char pixel[4] = {255, a_byte(c.r), a_byte(c.g), a_byte(c.b)};
            if (fwrite(pixel, 1, 4, f) != 4) { ok = 0; break; }
        }
    }
    if (fclose(f) != 0) ok = 0;
    return ok;
}
