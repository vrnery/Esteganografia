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
        printf("codificar [img base] [img secreta]\n");
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
    // Calcula a quantidade de pixels da imagem secreta
    int t_sec = secreta.width * secreta.height;
    for (int t=0; t<t_sec; t++) {
        // Codifica cada pixel da imagem secreta
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
    // Calcula o total de pixels limite para a imagem secreta
    long t_base = ((base->width * base->height) - 1) / 4;
    // Calcula o total de pixels da imagem secreta
    long t_secreta = secreta->width * secreta->height;
    // Compara se a imagem secreta cabe dentro da base
    if (t_secreta > t_base)
        return 0;
    else
        return 1;
}

// Validação dos pixels
int validacaoPixel(Img* secreta) {
    // Determina o tamanho maximo para a largura
    if (secreta->width > 4096)
        return 0;
    // Determina o tamanho maximo para a altura
    if (secreta->height > 4096)
        return 0;
    return 1;
}

// Zera 2 bitset
void bitwise2(Img* base) {
    // Calcula o total de pixels
    int xy = base->width * base->height;
    // Percorre todos os pixels
    for (int z=0; z<xy; z++) {
        // Utiliza a mascara 252 = 11111100 para escovar os bits
        base->img[z].r &= 252;
        base->img[z].g &= 252;
        base->img[z].b &= 252;
    }
}

// Codificação de largura e altura
void codificaTamanho(Img* base, int index, int tamanho) {
    // Subtrai 1 no tamanho, ex: 4096 - 1 = 4095 = 111111111111
    tamanho--;
    // Cria a mascara 3072 = 110000000000, desloca 10 bits para a direita
    // ex: 110000000000 >> 10 = 11
    // Armazena os 2 bits do tamanho (2^12)
    base->img[index].r |= ((tamanho & 3072) >> 10);
    // ex: 1100000000 >> 8 = 11
    // Armazena os 2 bits do tamanho (2^10)
    base->img[index].g |= ((tamanho & 768) >> 8);
    // ex: 11000000 >> 6 = 11
    // Armazena os 2 bits do tamanho (2^8)
    base->img[index].b |= ((tamanho & 192) >> 6);
    // Indexa para o proximo pixel da imagem base
    index++;
    // ex: 110000 >> 4 = 11
    // Armazena os 2 bits do tamanho (2^6)
    base->img[index].r |= ((tamanho & 48) >> 4);
    // ex: 1100 >> 2 = 11
    // Armazena os 2 bits do tamanho (2^4)
    base->img[index].g |= ((tamanho & 12) >> 2);
    // Armazena os 2 bits do tamanho (2^2)
    base->img[index].b |= (tamanho & 3);
}

// Codificar esteganografia
void codificaEsteganografia(Img* base, int index, RGB* pixel) {
    // Bits a deslocar
    int bit = 6;
    // Mascara 192 = 11000000
    int wise = 192;
    // Percorre 4 pixels apartir do informado (index)
    for (int x=index; x<(index+4); x++) {
        // Extrair os bits da imagem secreta usando a mascara
        // Deslocar bits extraidos para a direita (>> bit) sobrando apenas 2 bits
        // Escova 2 bits com o byte da imagem
        base->img[x].r |= ((pixel->r & wise) >> bit);
        base->img[x].g |= ((pixel->g & wise) >> bit);
        base->img[x].b |= ((pixel->b & wise) >> bit);
        // Divide a mascara ṕor 2 bits (11000000 >> 2 = 110000)
        wise >>= 2;
        // Diminui 2 bits de deslocamento (6 - 2 = 4)
        bit -= 2;
    }
}
