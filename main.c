#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <windows.h>

#define MAX_MUSICS 500

// ----------------- Struct que representa uma musica -----------------
typedef struct {
    char artista[200];
    char nome[200];
    char genero[200];
    char arquivo[300];
    float duracao;
} Musica;

// ----------------- Variaveis globais -----------------
Musica *musicas = NULL;
int totalMusicas = 0;
int capacidadeMusicas = 0;
char caminhoMusicas[500];
int pastaSelecionada = 0;

// ----------------- Garante capacidade no vetor dinamico -----------------
int garantirCapacidadeMusicas() {
    if (capacidadeMusicas == 0) {
        capacidadeMusicas = 50;
        musicas = (Musica *) malloc(capacidadeMusicas * sizeof(Musica));
        if (musicas == NULL) {
            printf("\nErro de alocacao de memoria para musicas.\n");
            capacidadeMusicas = 0;
            return 0;
        }
    } else if (totalMusicas >= capacidadeMusicas) {
        int novaCapacidade = capacidadeMusicas * 2;
        Musica *novo = (Musica *) realloc(musicas, novaCapacidade * sizeof(Musica));
        if (novo == NULL) {
            printf("\nErro ao realocar memoria para musicas.\n");
            return 0;
        }
        musicas = novo;
        capacidadeMusicas = novaCapacidade;
    }
    return 1;
}

// ----------------- Funcao para calcular duracao WAV -----------------
float obterDuracaoWav(const char *arquivo) {
    FILE *f = fopen(arquivo, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long tamanhoArquivo = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned int byteRate;
    fseek(f, 28, SEEK_SET);
    if (fread(&byteRate, sizeof(byteRate), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    if (byteRate == 0 || tamanhoArquivo <= 44) {
        fclose(f);
        return 0;
    }

    float duracao = (float)(tamanhoArquivo - 44) / byteRate;
    fclose(f);
    return duracao;
}

// ----------------- Salva arquivo de indice -----------------
void salvarIndice() {
    FILE *f = fopen("musicas_index.txt", "w");
    if (!f) {
        printf("Nao foi possivel salvar o indice (musicas_index.txt).\n");
        return;
    }

    for (int i = 0; i < totalMusicas; i++) {
        fprintf(f, "%s;%s;%s;%s;%.2f\n",
                musicas[i].artista,
                musicas[i].nome,
                musicas[i].genero,
                musicas[i].arquivo,
                musicas[i].duracao);
    }
    fclose(f);
}

// ----------------- Carrega musicas da pasta -----------------
void carregarMusicas() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(caminhoMusicas);
    if (!dir) {
        printf("\nErro ao abrir a pasta!\n");
        totalMusicas = 0;
        return;
    }

    totalMusicas = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".wav") != NULL || strstr(entry->d_name, ".WAV") != NULL) {

            if (!garantirCapacidadeMusicas()) {
                break;
            }

            char nomeArquivo[300];
            strcpy(nomeArquivo, entry->d_name);

            char *art = strtok(nomeArquivo, "-");
            char *nome = strtok(NULL, "-");
            char *gen = strtok(NULL, ".");

            if (art && nome && gen) {
                while (*art == ' ') art++;
                while (*nome == ' ') nome++;
                while (*gen == ' ') gen++;

                strcpy(musicas[totalMusicas].artista, art);
                strcpy(musicas[totalMusicas].nome, nome);
                strcpy(musicas[totalMusicas].genero, gen);
                sprintf(musicas[totalMusicas].arquivo, "%s\\%s", caminhoMusicas, entry->d_name);

                musicas[totalMusicas].duracao = obterDuracaoWav(musicas[totalMusicas].arquivo);
                totalMusicas++;
            }
        }
    }

    closedir(dir);
    salvarIndice();
    printf("\nMusicas carregadas e com indice atualizado (musicas_index.txt)!\n");
}

// ----------------- Cria pasta se nao existir -----------------
void criarPastaSeNaoExistir(const char *caminho) {
    if (CreateDirectory(caminho, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        // tudo certo: ou criou ou já existia
    } else {
        printf("Erro ao criar pasta: %s\n", caminho);
    }
}

// ----------------- Organiza musicas por genero -----------------
void organizarMusicasPorGenero() {
    if (!pastaSelecionada) {
        printf("\nPrimeiro selecione uma pasta!\n");
        return;
    }

    if (totalMusicas == 0) {
        printf("\nNao ha musicas carregadas para organizar.\n");
        return;
    }

    for (int i = 0; i < totalMusicas; i++) {
        char pasta[500];
        sprintf(pasta, "%s\\%s", caminhoMusicas, musicas[i].genero);

        criarPastaSeNaoExistir(pasta);

        char destino[500];
        sprintf(destino, "%s\\%s", pasta, strrchr(musicas[i].arquivo, '\\') + 1);

        if (!MoveFile(musicas[i].arquivo, destino)) {
            printf("Erro ao mover: %s\n", musicas[i].arquivo);
        } else {
            strcpy(musicas[i].arquivo, destino);
        }
    }

    salvarIndice();
    printf("\nMusicas organizadas por genero com sucesso!\n");
    printf("   As musicas foram movidas para pastas com o nome do genero\n");
    printf("   dentro da pasta base: %s\n", caminhoMusicas);
}

// ----------------- Seleciona pasta -----------------
void selecionarPastaInicial() {
    printf("=============================================\n");
    printf("      MINI GERENCIADOR DE MUSICAS WAV\n");
    printf("=============================================\n\n");

    printf("Formato esperado dos arquivos:\n");
    printf("  Artista - Nome da Musica - Genero.wav\n\n");

    printf("Digite o caminho da pasta de musicas (ex: C:\\\\MinhasMusicas):\n> ");

    if (fgets(caminhoMusicas, sizeof(caminhoMusicas), stdin) == NULL) {
        printf("\nCaminho invalido.\n");
        pastaSelecionada = 0;
        return;
    }
    caminhoMusicas[strcspn(caminhoMusicas, "\n")] = 0;

    if (strlen(caminhoMusicas) == 0) {
        printf("\nCaminho vazio. Programa sera encerrado.\n");
        pastaSelecionada = 0;
        return;
    }

    pastaSelecionada = 1;
    carregarMusicas();

    if (totalMusicas == 0) {
        printf("Nenhuma musica .wav encontrada nessa pasta.\n");
    }
}

// ----------------- Tocar musica -----------------
void tocarMusica() {
    if (!pastaSelecionada) {
        printf("\nPrimeiro selecione uma pasta!\n");
        return;
    }

    if (totalMusicas == 0) {
        printf("\nNao ha musicas carregadas.\n");
        return;
    }

    char generos[MAX_MUSICS][200];
    int qtdGeneros = 0;
    for (int i = 0; i < totalMusicas; i++) {
        int existe = 0;
        for (int j = 0; j < qtdGeneros; j++) {
            if (strcmp(generos[j], musicas[i].genero) == 0) {
                existe = 1;
                break;
            }
        }
        if (!existe && qtdGeneros < MAX_MUSICS) {
            strcpy(generos[qtdGeneros++], musicas[i].genero);
        }
    }

    if (qtdGeneros == 0) {
        printf("\nNenhum genero encontrado.\n");
        return;
    }

    printf("\nPlaylists disponiveis (generos):\n");
    for (int i = 0; i < qtdGeneros; i++) {
        printf("%d - %s\n", i + 1, generos[i]);
    }

    int escolhaGenero;
    printf("\nEscolha a playlist (genero): ");
    if (scanf("%d", &escolhaGenero) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    if (escolhaGenero < 1 || escolhaGenero > qtdGeneros) {
        printf("Opcao invalida!\n");
        return;
    }

    char generoEscolhido[200];
    strcpy(generoEscolhido, generos[escolhaGenero - 1]);

    printf("\nMusicas em %s:\n", generoEscolhido);
    int indices[MAX_MUSICS];
    int count = 0;
    for (int i = 0; i < totalMusicas; i++) {
        if (strcmp(musicas[i].genero, generoEscolhido) == 0) {
            if (count < MAX_MUSICS) {
                indices[count] = i;
                count++;
            }
            printf("%d - %s - %s\n", count, musicas[i].artista, musicas[i].nome);
        }
    }

    if (count == 0) {
        printf("\nNao ha musicas nesse genero.\n");
        return;
    }

    int escolhaMusica;
    printf("\nEscolha a musica: ");
    if (scanf("%d", &escolhaMusica) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    if (escolhaMusica < 1 || escolhaMusica > count) {
        printf("Opcao invalida!\n");
        return;
    }

    Musica m = musicas[indices[escolhaMusica - 1]];
    printf("\nTocando: %s - %s - %s\n", m.artista, m.nome, m.genero);

    ShellExecuteA(NULL, "open", m.arquivo, NULL, NULL, SW_SHOWNORMAL);
}

// ----------------- Gera relatorio -----------------
void gerarRelatorio() {
    if (!pastaSelecionada) {
        printf("\nPrimeiro selecione uma pasta!\n");
        return;
    }

    if (totalMusicas == 0) {
        printf("\nNao ha musicas carregadas para o relatorio.\n");
        return;
    }

    FILE *f = fopen("relatorio.txt", "w");
    if (!f) {
        printf("\nErro ao criar relatorio.txt\n");
        return;
    }

    fprintf(f, "=== RELATORIO DE MUSICAS ===\n");
    fprintf(f, "Pasta base       : %s\n", caminhoMusicas);
    fprintf(f, "Total de musicas : %d\n", totalMusicas);

    char generos[MAX_MUSICS][200];
    int qtdGeneros = 0;
    float duracaoTotal = 0;

    for (int i = 0; i < totalMusicas; i++) duracaoTotal += musicas[i].duracao;

    for (int i = 0; i < totalMusicas; i++) {
        int existe = 0;
        for (int j = 0; j < qtdGeneros; j++) {
            if (strcmp(generos[j], musicas[i].genero) == 0) {
                existe = 1;
                break;
            }
        }
        if (!existe && qtdGeneros < MAX_MUSICS) {
            strcpy(generos[qtdGeneros++], musicas[i].genero);
        }
    }

    fprintf(f, "Total de generos : %d\n", qtdGeneros);
    fprintf(f, "Duracao total    : %02d:%02d:%02d\n\n",
            (int)(duracaoTotal/3600),
            (int)((int)duracaoTotal%3600)/60,
            (int)((int)duracaoTotal%60));

    fprintf(f, "=== Por Genero ===\n");
    for (int i = 0; i < qtdGeneros; i++) {
        int count = 0;
        float durGenero = 0;
        for (int j = 0; j < totalMusicas; j++) {
            if (strcmp(generos[i], musicas[j].genero) == 0) {
                count++;
                durGenero += musicas[j].duracao;
            }
        }
        fprintf(f, "------------------------\n");
        fprintf(f, "%-15s\n", generos[i]);
        fprintf(f, "  Musicas       : %2d\n", count);
        fprintf(f, "  Duracao total : %02d:%02d:%02d\n",
                (int)(durGenero/3600),
                (int)((int)durGenero%3600)/60,
                (int)((int)durGenero%60));
    }
    fprintf(f, "------------------------\n");

    fclose(f);
    printf("\nRelatorio gerado: relatorio.txt\n");
}

// ----------------- MAIN -----------------
int main() {
    int opcao;

    selecionarPastaInicial();
    if (!pastaSelecionada) {
        return 0;
    }

    printf("Pasta atual: %s\n", caminhoMusicas);
    printf("Musicas carregadas: %d\n", totalMusicas);

    do {
        printf("------------------------\n");
        printf("1 - Organizar musicas por genero\n");
        printf("2 - Tocar musica\n");
        printf("3 - Gerar relatorio\n");
        printf("4 - Sair\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            break;
        }
        getchar(); // consome '\n'

        switch(opcao) {
            case 1:
                organizarMusicasPorGenero();
                break;
            case 2:
                tocarMusica();
                break;
            case 3:
                gerarRelatorio();
                break;
            case 4:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 4);

    if (musicas != NULL) {
        free(musicas);
    }

    return 0;
}
