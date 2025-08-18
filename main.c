#include "BMP.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int opcion = 0;
    char inPath[256];
    char outPath[256];

    BMPFILEHEADER fh;
    BMPINFOHEADER ih;

    // --- Paso 1: pedir archivo de entrada ---
    printf("(DEBG APGM) Ingrese el nombre del archivo BMP de entrada: ");
    scanf("%255s", inPath);

    unsigned char *data = cargarBMP(inPath, &fh, &ih);

    if (!data) {
        printf("Error al abrir la imagen\n");
        return 1;
    }

    if (ih.biBitCount != 24 && ih.biBitCount != 32) {
        printf("Solo se admite 24 o 32 bits por pixel (BMP sin compresion).\n");
        free(data);
        return 1;
    }

    printf("Imagen cargada: %dx%d, %d bits\n", ih.biWidth, ih.biHeight, ih.biBitCount);    

    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Convertir a escala de grises\n");
        printf("2. Aplicar convolucion\n");
        printf("3. Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);

        if (opcion == 3) {
            printf("Saliendo...\n");
            break;
        }

        // --- Paso 2: pedir archivo de salida ---
        printf("Ingrese el nombre del archivo BMP de salida: ");
        scanf("%255s", outPath);

        switch (opcion) {
        case 1:
            escalarGris(data, ih.biWidth, (ih.biHeight < 0 ? -ih.biHeight : ih.biHeight), ih.biBitCount);
            if (!guardar(outPath, &fh, &ih, data)) {
                printf("No se pudo guardar %s\n", outPath);
                free(data);
                return 1;
            }
            printf("Listo! Guardado en: %s\n", outPath);
            break;

        case 2: {
            printf("--- Convolucion ---\n");
            printf("Seleccione un kernel:\n");
            printf("1. Identidad\n");
            printf("2. Blur (suavizado)\n");
            printf("3. Sharpen (enfocar)\n");
            printf("4. Deteccion de bordes\n");
            printf("5. Emboss (relieve)\n");

            int ksel;
            scanf("%d", &ksel);

            int KERNEL[5][3][3] = {
                { {0,0,0}, {0,1,0}, {0,0,0} },             // Identidad
                { {1,1,1}, {1,1,1}, {1,1,1} },             // Blur
                { {0,-1,0}, {-1,5,-1}, {0,-1,0} },         // Sharpen
                { {-1,-1,-1}, {-1,8,-1}, {-1,-1,-1} },     // Bordes
                { {-2,-1,0}, {-1,1,1}, {0,1,2} }           // Emboss
            };

            int divisor = 1, offset = 0;
            switch (ksel) {
                case 1: divisor = 1; offset = 0; break;
                case 2: divisor = 9; offset = 0; break;
                case 3: divisor = 1; offset = 0; break;
                case 4: divisor = 1; offset = 0; break;
                case 5: divisor = 1; offset = 128; break;
                default: printf("Kernel invalido.\n"); continue;
            }

            aplicarConvolucion(data, ih.biWidth, (ih.biHeight < 0 ? -ih.biHeight : ih.biHeight),
                               ih.biBitCount, KERNEL[ksel-1], divisor, offset);

            if (!guardar(outPath, &fh, &ih, data)) {
                printf("No se pudo guardar %s\n", outPath);
                free(data);
                return 1;
            }
            printf("Listo! Imagen procesada y guardada en: %s\n", outPath);
            break;
        }

        default:
            printf("Opcion invalida.\n");
            break;
        }
    }

    free(data);
    return 0;
}
