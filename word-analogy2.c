//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries

int read_words(char st[100][max_size]);
int find_words (char st[100][max_size], long long *bi, char *vocab, long long words);
//int find_words (char st0[max_size], char st1[max_size], char st2[max_size], char st3[max_size], long long *bi, char *vocab, long long words);
int find_nearest (long long words, float *vec, char *vocab, float *M, long long *bi, long long size, float *bestd, char bestw[N][max_size]);

int main(int argc, char **argv) {
  FILE *f;
//  char st1[max_size];
  char bestw[N][max_size];
  char file_name[max_size], st[100][max_size];
  float dist, len, bestd[N], vec[max_size];
  long long words, size, a, b, c, d, cn, bi[100];
  float *M;
  char *vocab;
  if (argc < 2) {
    printf("Usage: ./word-analogy <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  strcpy(file_name, argv[1]);
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < max_w; a++) vocab[b * max_w + a] = toupper(vocab[b * max_w + a]);
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);


  while (1) {
    cn = read_words(st);
    if ( cn < 0 ) {
      printf("\nSAIDA\n");
      if ( feof(stdin) ) break;
      if ( !strcmp(st[0], "EXIT") ) break;
    }
    if ( cn > 0 ) {
      printf ("Palavras lidas: %s, %s, %s, %s\n", st[0], st[1], st[2], st[3]);

      if ( find_words(st, bi, vocab, words) <= 0 ) continue;

      find_nearest (words, vec, vocab, M, bi, size, bestd, bestw);

      printf("\n                                              Word              Distance\n------------------------------------------------------------------------\n");

      for (a = 0; a < N; a++) printf("%50s\t\t%f\n", bestw[a], bestd[a]);

    }

  }
  return 0;
}

//**********************
// Codigos de retorno:
// -1: Final da Execucao
// 0: Termino da leitura de palavras no Grupo
// >0: Quantidade de palavras lidas.
int read_words(char st[100][max_size]) {
  long long a, cn;

  // Leitura da primeira palavra e validacao das condicoes de saida
  scanf("%s", st[0]);
  for (a = 0; a < strlen(st[0]); a++) st[0][a] = toupper(st[0][a]);
  if ((!strcmp(st[0], ":")) || (!strcmp(st[0], "EXIT")) || feof(stdin)) {
    if ( (!strcmp(st[0], "EXIT")) || (feof(stdin)) ) return -1;
    scanf("%s", st[0]);
    if ( (!strcmp(st[0], "EXIT")) || (feof(stdin)) ) return -1;
    return 0;
  }

  // Leitura das palavras seguintes
  for (cn = 1; cn < 4; cn++) {
    scanf("%s", st[cn]);
    if ((!strcmp(st[cn], ":")) || (!strcmp(st[cn], "EXIT")) || feof(stdin)) {
      printf("\nERRO - Foram lidas %lld palavras, enquanto o esperado eram 4.\n", cn);
      return -1;
    }
    for (a = 0; a < strlen(st[cn]); a++) st[cn][a] = toupper(st[cn][a]);
  }

  return 1;

}


int find_words (char st[100][max_size], long long *bi, char *vocab, long long words) {
//  long long a = 0, b = 0, cn = 0;
  long long cn, b;

  b = cn = 0;

  // Procura pelas palavras no dicionario
  for (cn = 0; cn < 4; cn++) {
    for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[cn])) break;
    bi[cn] = b;
    if (bi[cn] == words) {
      printf("Out of dictionary word!\n");
      bi[cn] = 0;
      return 0;
    }
//    else
//      printf("\nWord: %s  Position in vocabulary: %lld\n", st[cn], bi[cn]);
  }

  return 1;
}


int find_nearest (long long words, float *vec, char *vocab, float *M, long long *bi, long long size, float *bestd, char bestw[N][max_size]) {
  long long a, b, c, d;
  float len, dist;

  // Calculando o vetor resultante
  for (a = 0; a < size; a++) vec[a] = M[a + bi[1] * size] - M[a + bi[0] * size] + M[a + bi[2] * size];
  // Normalizando o vetor
  len = 0;
  for (a = 0; a < size; a++) len += vec[a] * vec[a];
  len = sqrt(len);
  for (a = 0; a < size; a++) vec[a] /= len;

  for (a = 0; a < N; a++) bestd[a] = 0;
  for (a = 0; a < N; a++) bestw[a][0] = 0;
  for (c = 0; c < words; c++) {
    if (c == bi[0]) continue;
    if (c == bi[1]) continue;
    if (c == bi[2]) continue;
    a = 0;
    for (b = 0; b < 3; b++) if (bi[b] == c) a = 1;
    if (a == 1) continue;
    dist = 0;
    for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
    for (a = 0; a < N; a++) {
      if (dist > bestd[a]) {
        for (d = N - 1; d > a; d--) {
          bestd[d] = bestd[d - 1];
          strcpy(bestw[d], bestw[d - 1]);
        }
        bestd[a] = dist;
        strcpy(bestw[a], &vocab[c * max_w]);
        break;
      }
    }
  }
  return 1;
}
