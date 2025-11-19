/*
 * Projeto 2 - Codificador de Imagens Binárias
 * Disciplina: Algoritmos e Programação II
 *
 * GRUPO:
 *   10740428 - Henrique Cunha Alves
 *   10438660 - Gustavo Francisco Toito
 *   10736785 - Guilherme Longo Gouveia Xavier
 *
 * Link do vídeo no YouTube:
 *    
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LARGURA 1024
#define MAX_ALTURA  768

/* Protótipos */
void exibir_uso(const char *progname);
int ler_imagem_manual(int pixels[][MAX_LARGURA], int *largura, int *altura);
int ler_imagem_pbm(const char *nome_arquivo, int pixels[][MAX_LARGURA],
                   int *largura, int *altura);
int regiao_uniforme(int pixels[][MAX_LARGURA],
                    int x, int y, int w, int h, int *valor);
void codificar_regiao(int pixels[][MAX_LARGURA],
                      int x, int y, int w, int h);

/* Função principal */
int main(int argc, char *argv[]) {
    int pixels[MAX_ALTURA][MAX_LARGURA];
    int largura = 0, altura = 0;

    if (argc == 1) {
        /*Nenhum parametro ira mostrar o help*/
        exibir_uso(argv[0]);
        return 0;
    }

    /*opções da linha de comando*/
    if (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "--help") == 0) {
        exibir_uso(argv[0]);
        return 0;
    } else if (strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--manual") == 0) {
        /* Modo manual */
        if (!ler_imagem_manual(pixels, &largura, &altura)) {
            fprintf(stderr, "Erro ao ler imagem em modo manual.\n");
            return 1;
        }
    } else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0) {
        /* modo do arquivo PBM */
        if (argc < 3) {
            fprintf(stderr, "Erro: é necessário informar o nome do arquivo PBM.\n");
            exibir_uso(argv[0]);
            return 1;
        }
        if (!ler_imagem_pbm(argv[2], pixels, &largura, &altura)) {
            fprintf(stderr, "Erro ao ler arquivo PBM '%s'.\n", argv[2]);
            return 1;
        }
    } else {
        fprintf(stderr, "Opcao invalida: %s\n", argv[1]);
        exibir_uso(argv[0]);
        return 1;
    }

    /* Codificar a imagem inteira (0,0) até largura e altura desejadas(com limites)*/
    codificar_regiao(pixels, 0, 0, largura, altura);
    putchar('\n');

    return 0;
}

/*Função de ajuda*/
void exibir_uso(const char *progname) {
    printf("Uso: %s [-? | -m | -f ARQ]\n", progname);
    printf("Codifica imagens binarias dadas em arquivos PBM ou por dados informados manualmente.\n\n");
    printf("Argumentos:\n");
    printf("  -?, --help    : apresenta esta orientacao na tela.\n");
    printf("  -m, --manual  : ativa o modo de entrada manual (dados via teclado).\n");
    printf("  -f, --file ARQ: considera a imagem representada no arquivo PBM (Portable bitmap).\n");
}

/*Modo manual */
int ler_imagem_manual(int pixels[][MAX_LARGURA], int *largura, int *altura) {
    int i, j;
    printf("Modo manual selecionado.\n");
    printf("Informe a largura (max %d): ", MAX_LARGURA);
    if (scanf("%d", largura) != 1) return 0;
    printf("Informe a altura (max %d): ", MAX_ALTURA);
    if (scanf("%d", altura) != 1) return 0;

    if (*largura <= 0 || *largura > MAX_LARGURA ||
        *altura <= 0 || *altura > MAX_ALTURA) {
        fprintf(stderr, "Dimensoes invalidas (largura ou altura fora do limite).\n");
        return 0;
    }

    printf("Informe os pixels da imagem (0 = branco, 1 = preto).\n");
    printf("Total de %d linhas x %d colunas.\n", *altura, *largura);

    for (i = 0; i < *altura; i++) {
        for (j = 0; j < *largura; j++) {
            int v;
            if (scanf("%d", &v) != 1) {
                fprintf(stderr, "Erro de leitura dos pixels.\n");
                return 0;
            }
            if (v != 0 && v != 1) {
                fprintf(stderr, "Valor de pixel invalido em (%d,%d): %d. Use 0 ou 1.\n",
                        i, j, v);
                return 0;
            }
            pixels[i][j] = v;
        }
    }

    return 1;
}

/*Leitura de arquivo PBM so P1 */
static void pular_espacos_e_comentarios(FILE *f) {
    int c;
    while (1) {
        c = fgetc(f);
        /*Pula os espaços em branco*/
        while (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
            c = fgetc(f);
        }
        if (c == '#') {
            /*Linha de comentario vai ler ate fim da linha*/
            while (c != '\n' && c != EOF) {
                c = fgetc(f);
            }
            /*volta pro laço pois ainda pode ter comentario e espacos*/
        } else {
            if (c != EOF) {
                ungetc(c, f);
            }
            break;
        }
    }
}

int ler_imagem_pbm(const char *nome_arquivo, int pixels[][MAX_LARGURA],
                   int *largura, int *altura) {
    FILE *fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror("Nao foi possivel abrir o arquivo");
        return 0;
    }

    char magic[3];
    if (fscanf(fp, "%2s", magic) != 1) {
        fprintf(stderr, "Erro ao ler magic number do PBM.\n");
        fclose(fp);
        return 0;
    }

    if (strcmp(magic, "P1") != 0) {
        fprintf(stderr, "Formato nao suportado. Esperado P1, encontrado '%s'.\n", magic);
        fclose(fp);
        return 0;
    }

    /*Pular espacos e comentarios ate encontrar largura e altura*/
    pular_espacos_e_comentarios(fp);

    if (fscanf(fp, "%d %d", largura, altura) != 2) {
        fprintf(stderr, "Erro ao ler largura e altura do PBM.\n");
        fclose(fp);
        return 0;
    }

    if (*largura <= 0 || *largura > MAX_LARGURA ||
        *altura <= 0 || *altura > MAX_ALTURA) {
        fprintf(stderr, "Dimensoes do PBM fora do limite suportado (%dx%d).\n",
                MAX_LARGURA, MAX_ALTURA);
        fclose(fp);
        return 0;
    }

    /*ler pixels (0 = branco  1 = preto)*/
    int i, j, v;
    for (i = 0; i < *altura; i++) {
        for (j = 0; j < *largura; j++) {
            if (fscanf(fp, "%d", &v) != 1) {
                fprintf(stderr, "Erro ao ler pixel (%d,%d) do PBM.\n", i, j);
                fclose(fp);
                return 0;
            }
            if (v != 0 && v != 1) {
                fprintf(stderr, "Pixel invalido (%d,%d) no PBM: %d (esperado 0 ou 1).\n",
                        i, j, v);
                fclose(fp);
                return 0;
            }
            pixels[i][j] = v;
        }
    }

    fclose(fp);
    return 1;
}

/*Verifica se regiao e uniforme*/
int regiao_uniforme(int pixels[][MAX_LARGURA],
                    int x, int y, int w, int h, int *valor) {
    int i, j;
    int primeiro = pixels[y][x];

    for (i = y; i < y + h; i++) {
        for (j = x; j < x + w; j++) {
            if (pixels[i][j] != primeiro) {
                return 0;  /* nao uniforme */
            }
        }
    }

    *valor = primeiro;
    return 1;  /* uniforme */
}

/*Codificação recursiva*/
void codificar_regiao(int pixels[][MAX_LARGURA],
                      int x, int y, int w, int h) {
    int valor;

    /*Seguranca se a regiao tiver dimensao nula, nao faz nada*/
    if (w <= 0 || h <= 0) {
        return;
    }

    /*Testa se a regiao e uniforme*/
    if (regiao_uniforme(pixels, x, y, w, h, &valor)) {
        /*0 = branco (B) 1 = preto (P)*/
        if (valor == 0)
            putchar('B');
        else
            putchar('P');
        return;
    }

    /* Nao uniforme dividir em subimagens e emitir 'X' */
    putchar('X');

    /*Calculo dos tamanhos dos quadrantes*/
    int w1, w2, h1, h2;

    if (w > 1) {
        w1 = (w + 1) / 2;   /*lado esquerdo (coluna extra se w for impar)*/
        w2 = w - w1;        /*lado direito*/
    } else {
        w1 = w;
        w2 = 0;
    }

    if (h > 1) {
        h1 = (h + 1) / 2;   /*parte de cima (linha extra se h for impar)*/
        h2 = h - h1;        /*parte de baixo*/
    } else {
        h1 = h;
        h2 = 0;
    }

    /*primeiro quadrante: superior esquerdo*/
    if (w1 > 0 && h1 > 0)
        codificar_regiao(pixels, x, y, w1, h1);

    /*segundo quadrante: superior direito*/
    if (w2 > 0 && h1 > 0)
        codificar_regiao(pixels, x + w1, y, w2, h1);

    /*terceiro quadrante: inferior esquerdo*/
    if (w1 > 0 && h2 > 0)
        codificar_regiao(pixels, x, y + h1, w1, h2);

    /*quarto quadrante: inferior direito*/
    if (w2 > 0 && h2 > 0)
        codificar_regiao(pixels, x + w1, y + h1, w2, h2);
}
