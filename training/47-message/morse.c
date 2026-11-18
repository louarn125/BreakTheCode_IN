// gcc morse.c -o morse -lm   puis   ./morse message.wav 700
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char *L[] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--.."};
static const char *D[] = {"-----",".----","..---","...--","....-",".....","-....","--...","---..","----."};

static char lookup(const char *s) {
    for (int i = 0; i < 26; i++) if (!strcmp(s, L[i])) return 'A' + i;
    for (int i = 0; i < 10; i++) if (!strcmp(s, D[i])) return '0' + i;
    return '?';
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: %s message.wav freq_hz\n", argv[0]); return 1; }
    double freq = atof(argv[2]);
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("open"); return 1; }

    char id[4]; uint32_t size;
    fread(id, 1, 4, f); fread(&size, 4, 1, f); fread(id, 1, 4, f);
    uint16_t ch = 1, bits = 16; uint32_t rate = 44100, dsize = 0;
    uint8_t *raw = NULL;
    while (fread(id, 1, 4, f) == 4 && fread(&size, 4, 1, f) == 1) {
        if (!memcmp(id, "fmt ", 4)) {
            uint16_t fmt; fread(&fmt, 2, 1, f); fread(&ch, 2, 1, f);
            fread(&rate, 4, 1, f); fseek(f, 6, SEEK_CUR); fread(&bits, 2, 1, f);
            fseek(f, size - 16, SEEK_CUR);
        } else if (!memcmp(id, "data", 4)) {
            raw = malloc(size); dsize = fread(raw, 1, size, f); break;
        } else fseek(f, size + (size & 1), SEEK_CUR);
    }
    fclose(f);
    if (!raw || bits != 16) { fprintf(stderr, "WAV PCM 16 bits requis\n"); return 1; }

    int16_t *s = (int16_t *)raw;
    size_t n = dsize / 2 / ch;                 // échantillons par canal
    int win = rate / 200;                      // fenêtres de 5 ms
    size_t nw = n / win;
    double *mag = malloc(nw * sizeof(double)), maxv = 0;
    double coef = 2 * cos(2 * M_PI * freq / rate);

    for (size_t w = 0; w < nw; w++) {
        double s1 = 0, s2 = 0;
        for (int i = 0; i < win; i++) {
            double s0 = s[(w * win + i) * ch] + coef * s1 - s2;
            s2 = s1; s1 = s0;
        }
        mag[w] = s1 * s1 + s2 * s2 - coef * s1 * s2;
        if (mag[w] > maxv) maxv = mag[w];
    }

    // runs allumé/éteint
    int *len = malloc(nw * sizeof(int)), *on = malloc(nw * sizeof(int)); int nr = 0;
    int cur = mag[0] > 0.3 * maxv, cnt = 0;
    for (size_t w = 0; w < nw; w++) {
        int st = mag[w] > 0.3 * maxv;
        if (st == cur) cnt++;
        else { len[nr] = cnt; on[nr++] = cur; cur = st; cnt = 1; }
    }
    len[nr] = cnt; on[nr++] = cur;

    // estimation point/trait (2 classes) sur les durées "allumé" (>= 2 fenêtres)
    double lo = 1e9, hi = 0;
    for (int i = 0; i < nr; i++) if (on[i] && len[i] >= 2) {
        if (len[i] < lo) lo = len[i];
        if (len[i] > hi) hi = len[i];
    }
    for (int it = 0; it < 10; it++) {
        double sl = 0, sh = 0; int cl = 0, ch2 = 0;
        for (int i = 0; i < nr; i++) if (on[i] && len[i] >= 2) {
            if (fabs(len[i] - lo) <= fabs(len[i] - hi)) { sl += len[i]; cl++; }
            else { sh += len[i]; ch2++; }
        }
        if (cl) lo = sl / cl;
        if (ch2) hi = sh / ch2;
    }

        for (int i = 0; i < nr && i < 80; i++){
            fprintf(stderr, "%s %d ms\n", on[i] ? "ON " : "off", len[i] * 5);

        }

    double dot = lo, cut = (lo + hi) / 2;
    fprintf(stderr, "point ~ %.0f ms, trait ~ %.0f ms\n", dot * 5, hi * 5);

    char sym[16] = ""; int k = 0;
    printf("Morse : ");
    char text[4096]; int t = 0;
    for (int i = 0; i < nr; i++) {
        if (on[i]) {
            if (len[i] < 2) continue;                  // parasite
            char c = len[i] < cut ? '.' : '-';
            if (k < 15) { sym[k++] = c; sym[k] = 0; }
            putchar(c);
        } else if (k) {
            if (len[i] >= 1.5 * dot) {                   // fin de lettre
                text[t++] = lookup(sym); k = 0; sym[0] = 0; putchar(' ');
                if (len[i] >= 3 * dot) { text[t++] = ' '; printf("/ "); }
            }
        }
    }
    if (k) text[t++] = lookup(sym);
    text[t] = 0;
    printf("\nTexte : %s\n", text);
    return 0;
}