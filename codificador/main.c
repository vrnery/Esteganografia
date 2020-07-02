#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// Para usar strings

// SOIL é a biblioteca para leitura das imagens
#include "include/SOIL.h"

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
int validacaoImagem(Img* base, Img* secreta);
int validacaoPixel(Img* secreta);
void bitwise2(Img* base);
void codificaTamanho(Img* base, int index, int tamanho);
void codificaEsteganografia(Img* base, int index, RGB* pixel);

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
    if(argc < 3) {
        printf("loader [img base] [img secreta]\n");
        exit(1);
    }
    load(argv[1], &base);
    load(argv[2], &secreta);

    // Verificando imagens
    if (!validacaoImagem(&base, &secreta)) {
        printf("imagem base não suporta a imagem secreta\n");
        exit(1);
    }

    // Verificando width e heigth
    if (!validacaoPixel(&secreta)) {
        printf("imagem secreta com resolução maior que 4096 pixels de largura ou altura\n");
        exit(1);
    }

    printf("Primeiros 10 pixels da imagem base:\n");
    for(int i=0; i<10; i++)
        printf("[%02X %02X %02X] ", base.img[i].r, base.img[i].g, base.img[i].b);
    printf("\n\n");

    printf("Primeiros 10 pixels da imagem secreta:\n");
    for(int i=0; i<10; i++)
        printf("[%02X %02X %02X] ", secreta.img[i].r, secreta.img[i].g, secreta.img[i].b);
    printf("\n\n");

    // Zerar bits menos significativos
    bitwise2(&base);

    // Codifica a largura;
    codificaTamanho(&base, 0, secreta.width);
    codificaTamanho(&base, 2, secreta.height);

    // Esteganograma;
    int t_sec = secreta.width * secreta.height;
    for (int t=0; t<t_sec; t++) {
        codificaEsteganografia(&base, ((t * 4) + 4), &secreta.img[t]);
    }

    printf("Teste: gravando imagem base em saida.bmp\n");
    SOIL_save_image("saida.bmp", SOIL_SAVE_TYPE_BMP,
     base.width, base.height, 3, (const unsigned char*) base.img);

    free(base.img);
    free(secreta.img);
}

// Validação das imagens
int validacaoImagem(Img* base, Img* secreta) {
    long t_base = ((base->width * base->height) - 1) / 4;
    long t_secreta = secreta->width * secreta->height;
    if (t_secreta > t_base)
        return 0;
    else
        return 1;
}

// Validação dos pixels
int validacaoPixel(Img* secreta) {
    if (secreta->width > 4096)
        return 0;
    if (secreta->height > 4096)
        return 0;
    return 1;
}

// Zera 2 bitset
void bitwise2(Img* base) {
    int xy = base->width * base->height;
    for (int z=0; z<xy; z++) {
        base->img[z].r &= 252;
        base->img[z].g &= 252;
        base->img[z].b &= 252;
    }
}

// Codificação de largura e altura
void codificaTamanho(Img* base, int index, int tamanho) {
    tamanho--;
    base->img[index].r |= ((tamanho & 3072) >> 10);
    base->img[index].g |= ((tamanho & 768) >> 8);
    base->img[index].b |= ((tamanho & 192) >> 6);
    index++;
    base->img[index].r |= ((tamanho & 48) >> 4);
    base->img[index].g |= ((tamanho & 12) >> 2);
    base->img[index].b |= (tamanho & 3);
}

// Codificar esteganografia
void codificaEsteganografia(Img* base, int index, RGB* pixel) {
    int bit = 6;
    int wise = 192;
    for (int x=index; x<(index+4); x++) {
        base->img[x].r |= ((pixel->r & wise) >> bit);
        base->img[x].g |= ((pixel->g & wise) >> bit);
        base->img[x].b |= ((pixel->b & wise) >> bit);
        wise >>= 2;
        bit -= 2;
    }
}
