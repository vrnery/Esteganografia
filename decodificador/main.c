#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// Para usar strings

// SOIL é a biblioteca para leitura das imagens
#include "include/SOIL.h"

// Int
#include <stdint.h>

// Um uint12 12bits
typedef struct {
    unsigned int valor : 12;
} uint12;

// Um pixel RGB
typedef struct {
    unsigned char r, g, b;
} RGB;

// Uma imagem em RGB
typedef struct {
    int width, height;
    RGB* img;
} Img;

// Protótipos
void load(char* name, Img* pic);
void decodificaTamanho(Img* base, int index, uint12* tamanho);
void bitwise8(Img* secreta);
void decodificaEsteganografia(Img* base, int index, RGB* pixel);

// Carrega uma imagem para a struct Img
void load(char* name, Img* pic)
{
    int chan;
    pic->img = (RGB*) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if(!pic->img)
    {
        printf( "SOIL loading error: %s (%s)\n", SOIL_last_result(), name);
        exit(1);
    }
    printf("Load: %s (%d x %d x %d)\n", name, pic->width, pic->height, chan);
}

int main(int argc, char** argv)
{
    Img base, secreta;
    if(argc < 2) {
        printf("decodificador [img base]\n");
        exit(1);
    }
    load(argv[1], &base);

    printf("Primeiros 10 pixels da imagem base:\n");
    for(int i=0; i<10; i++)
        printf("[%02X %02X %02X] ", base.img[i].r, base.img[i].g, base.img[i].b);
    printf("\n\n");

    // Decodifica a largura;
    uint12 width;
    width.valor = 0;
    uint12 height;
    height.valor = 0;
    decodificaTamanho(&base, 0, &width);
    decodificaTamanho(&base, 2, &height);
    secreta.width = width.valor;
    secreta.height = height.valor;
    secreta.img = calloc(secreta.width * secreta.height, 3);
    printf("width: %d | height: %d | pixel: %d\n", secreta.width, secreta.height, (secreta.width * secreta.height));

    // Zerar bits
    bitwise8(&secreta);

    // Esteganograma;
    int t_sec = secreta.width * secreta.height;
    for (int t=0; t<t_sec; t++) {
        decodificaEsteganografia(&base, ((t * 4) + 4), &secreta.img[t]);
    }

    printf("Teste: gravando imagem base em secreta.bmp\n");
    SOIL_save_image("secreta.bmp", SOIL_SAVE_TYPE_BMP,
     secreta.width, secreta.height, 3, (const unsigned char*) secreta.img);

    free(base.img);
    free(secreta.img);
}

// Decodificação de largura e altura
void decodificaTamanho(Img* base, int index, uint12* tamanho) {
    tamanho->valor |= ((base->img[index].r & 3) << 10);
    tamanho->valor |= ((base->img[index].g & 3) << 8);
    tamanho->valor |= ((base->img[index].b & 3) << 6);
    index++;
    tamanho->valor |= ((base->img[index].r & 3) << 4);
    tamanho->valor |= ((base->img[index].g & 3) << 2);
    tamanho->valor |= (base->img[index].b & 3);
    tamanho->valor++;
}

// Zera 8 bitset
void bitwise8(Img* secreta) {
    int xy = secreta->width * secreta->height;
    for (int z=0; z<xy; z++) {
        secreta->img[z].r = 0;
        secreta->img[z].g = 0;
        secreta->img[z].b = 0;
    }
}

// Decodificar esteganografia
void decodificaEsteganografia(Img* base, int index, RGB* pixel) {
    int bit = 6;
    int wise = 3;
    for (int x=index; x<(index+4); x++) {
        pixel->r |= ((base->img[x].r & wise) << bit);
        pixel->g |= ((base->img[x].g & wise) << bit);
        pixel->b |= ((base->img[x].b & wise) << bit);
        bit -= 2;
    }
}
