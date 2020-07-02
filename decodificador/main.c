#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// Para usar strings

// SOIL é a biblioteca para leitura das imagens
#include "include/SOIL.h"

// Um uint12 12 bits
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

    // Decodifica a largura e altura;
    uint12 width;
    uint12 height;
    // Inicializa a largura
    width.valor = 0;
    // Inicializa a altura
    height.valor = 0;
    // Carrega a largura
    decodificaTamanho(&base, 0, &width);
    // Carrega a altura
    decodificaTamanho(&base, 2, &height);
    // Define a largura e altura da imagem secreta
    secreta.width = width.valor;
    secreta.height = height.valor;
    // Reserva a quantidade de memoria para os pixels
    secreta.img = calloc(secreta.width * secreta.height, 3);
    printf("width: %d | height: %d | pixel: %d\n", secreta.width, secreta.height, (secreta.width * secreta.height));

    // Zerar bits da imagem secreta
    bitwise8(&secreta);

    // Esteganograma;
    // Calcula a quantidade de pixels da imagem secreta
    int t_sec = secreta.width * secreta.height;
    for (int t=0; t<t_sec; t++) {
        // Decodifica cada pixel da imagem secreta
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
    // Cria a mascara 3 = 11, desloca 10 bits para a esquerda
    // ex: 11 << 10 = 110000000000
    // Escova os 12 bits com o tamanho (2^12)
    tamanho->valor |= ((base->img[index].r & 3) << 10);
    // ex: 11 << 8 = 1100000000
    // Escova os 10 bits com o tamanho (2^10)
    tamanho->valor |= ((base->img[index].g & 3) << 8);
    // ex: 11 << 6 = 11000000
    // Escova os 8 bits com o tamanho (2^8)
    tamanho->valor |= ((base->img[index].b & 3) << 6);
    // Indexa para o proximo pixel da imagem base
    index++;
    // ex: 11 << 4 = 110000
    // Escova os 6 bits com o tamanho (2^6)
    tamanho->valor |= ((base->img[index].r & 3) << 4);
    // ex: 11 << 2 = 1100
    // Escova os 4 bits com o tamanho (2^4)
    tamanho->valor |= ((base->img[index].g & 3) << 2);
    // Escova os 2 bits com o tamanho (2^2)
    tamanho->valor |= (base->img[index].b & 3);
    // Adiciona 1 no tamanho, ex: 4095 + 1 = 4096 = 1000000000000
    tamanho->valor++;
}

// Zera 8 bitset
void bitwise8(Img* secreta) {
    // Calcula o total de pixels
    int xy = secreta->width * secreta->height;
    // Percorre todos os pixels
    for (int z=0; z<xy; z++) {
        // Armazena 00000000
        secreta->img[z].r = 0;
        secreta->img[z].g = 0;
        secreta->img[z].b = 0;
    }
}

// Decodificar esteganografia
void decodificaEsteganografia(Img* base, int index, RGB* pixel) {
    // Bits a deslocar
    int bit = 6;
    // Mascara 3 = 11
    int wise = 3;
    // Percorre 4 pixels apartir do informado (index)
    for (int x=index; x<(index+4); x++) {
        // Extrair os bits da imagem base usando a mascara
        // Deslocar os bits extraidos para a esquerda (<< bit)
        // Excovar os bits com o byte da imagem secreta
        pixel->r |= ((base->img[x].r & wise) << bit);
        pixel->g |= ((base->img[x].g & wise) << bit);
        pixel->b |= ((base->img[x].b & wise) << bit);
        // Diminui 2 bits de deslocamento (6 - 2 = 4)
        bit -= 2;
    }
}
