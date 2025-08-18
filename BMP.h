#ifndef BMP_H
#define BMP_H

#pragma pack(push, 1)

typedef struct 
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1, bfReserved2;
    unsigned int bfOffBits; 
} BMPFILEHEADER;

typedef struct
{
    unsigned int biSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter, biYPelsPerMeter;
    unsigned int biCirUsed;
    unsigned int biCirImportant;
} BMPINFOHEADER;

#pragma pack(pop)

unsigned char *cargarBMP(const char *imagen, BMPFILEHEADER *fh, BMPINFOHEADER *ih);

void escalarGris(unsigned char *data, int ancho, int alto, int bpp);

int guardar(const char *archivoGuardar, const BMPFILEHEADER *infh, const BMPINFOHEADER *inIh, const unsigned char *data);//se ponen como constantes porque queremos qcrear un archivo con valores que no se modifiquen

void aplicarConvolucion(unsigned char *data, int ancho, int alto, int bpp, const int kernel[3][3], int divisor, int offset);

#endif
