#include "BMP.h"

#include <stdlib.h>
#include <stdio.h>

unsigned char *cargarBMP(const char *imagen, BMPFILEHEADER *fh, BMPINFOHEADER *ih){

    FILE *f = fopen(imagen, "rb");// un archivo file se crea abriendo o accediendo a un archivo (dandole ese valor)
    if(!f) return NULL;

    fread(fh, sizeof(BMPFILEHEADER), 1, f);
    fread(ih, sizeof(BMPINFOHEADER), 1, f);

    int bpp = (ih->biBitCount)/8;
    int tFila = (ih->biWidth * bpp + 3) & (~3);//el ~3 es forzar un numero que sea multiplo de 4
    unsigned char *data = (unsigned char*)malloc(tFila * ih->biHeight);//ayuda a guardar los bytes de los colores sabiendo el tamanio de la imagen

    fseek(f, fh->bfOffBits, SEEK_SET);
    fread(data, tFila, ih->biHeight, f);

    fclose(f);
    return data;
}

void escalarGris(unsigned char *data, int ancho, int alto, int bpp){

    int bypp = bpp/8; //detecto los bytes de el archivo original
    int tFila = (ancho * bypp + 3) & (~3);// se da a multiplos de 4

    if (bypp < 3) return; //hace que solo se pueda con imagenes de 24 0 32 bits

    for (int j = 0; j < alto; j++)
    {
        unsigned char *fila = data + j * tFila;
        for (int i = 0; i < ancho; i++)
        {
            unsigned char *pixel = fila + i * bypp;

            unsigned char R = pixel[2], G = pixel[1], B = pixel[0];

            unsigned int gris = (77u * R + 150u * G + 29u * B + 128u) >> 8; //(77,150,29)/256 se acomoda con bitwise para poder tener el gris segun la ecuacion que utilice
                                                                           //se pone la u en el numero para indicar que no tiene signo
            pixel[0] = (unsigned char)gris; //hacemos que el gris solo ocupe 1 byte
            pixel[1] = (unsigned char)gris;
            pixel[2] = (unsigned char)gris;
        }
        
    }
    
}

int guardar(const char *nombreArchivo, const BMPFILEHEADER *inFh, const BMPINFOHEADER *inIh, const unsigned char *data){

    FILE *f = fopen(nombreArchivo, "wb");
    if(!f) return 0;

    BMPFILEHEADER fh = *inFh;
    BMPINFOHEADER ih = *inIh;

    int bypp = ih.biBitCount / 8;
    if(bypp < 3) {fclose(f); return 0;}

    int ancho = ih.biWidth;
    int alto = ih.biHeight;
    int tFila = (ancho * bypp + 3) & (~3);
    unsigned int tImagen = tFila * (alto < 0 ? -alto : alto);

    ih.biCompression = 0;             // BI_RGB sin compresión
    ih.biSizeImage = tImagen;

    if (fh.bfOffBits < sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER))
    {
        fh.bfOffBits = sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER);//asegurar el valor del offset de la imagen
    }

    fh.bfSize = fh.bfOffBits + tImagen;

    fwrite(&fh, sizeof(BMPFILEHEADER), 1, f); // se escriben los headers de la imagen nueva
    fwrite(&ih, sizeof(BMPINFOHEADER), 1, f);

    long posicion = ftell(f);
    for (long i = posicion; i < (long)fh.bfOffBits; ++i) fputc(0, f); //se rellenan con 0 si el offset de bits es mayor para evcitar errores

    size_t escrito = fwrite(data, 1, tImagen, f);
    fclose(f);
    
    return escrito == tImagen;//retornamos si el archivo nuevo cuenta con los valores originakes de la imagen en tamanio de ancho y alto
}
#include <string.h>

// Helpers para convolución
static inline unsigned char clampToByte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}

void aplicarConvolucion(unsigned char *data, int ancho, int alto, int bpp,
                        const int kernel[3][3], int divisor, int offset) {
    int bypp = bpp / 8;
    int tFila = (ancho * bypp + 3) & (~3); //calcula que el ancho de la imagen sea el real en bytes

    unsigned char *copy = (unsigned char*)malloc(tFila * alto); //separa un espacio de memoria para guardar la copia
    memcpy(copy, data, tFila * alto); //lo copia en el espacio que le guardanos antes con memory allocation

    for (int y = 1; y < alto - 1; y++) {
        for (int x = 1; x < ancho - 1; x++) {
            int sumR = 0, sumG = 0, sumB = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    unsigned char *p = copy + (y + ky) * tFila + (x + kx) * bypp; //el pixel se modifica y se almacena en la matriz segun si es verde rojo u azul
                    int kval = kernel[ky + 1][kx + 1];

                    sumB += p[0] * kval;
                    sumG += p[1] * kval;
                    sumR += p[2] * kval;
                }
            }

            if (divisor == 0) divisor = 1;
            sumR = sumR / divisor + offset;
            sumG = sumG / divisor + offset;
            sumB = sumB / divisor + offset;

            unsigned char *pixel = data + y * tFila + x * bypp; //se aplican los cambios en los pixeles de la nueva imagen con los cambios de kernel
            pixel[0] = clampToByte(sumB);
            pixel[1] = clampToByte(sumG);
            pixel[2] = clampToByte(sumR);
        }
    }

    free(copy);
}