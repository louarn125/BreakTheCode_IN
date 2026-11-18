#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include "fichier.h"

#define TAILLE_TEXT 256

typedef struct code{
    char* code;
    int n;
} code;

char* charger_texte(const char* nom_fichier){

    FILE * fichier = fopen(nom_fichier,"r");
    assert(fichier != NULL);
    if(fichier == NULL){
        return NULL;
    }

    char * text = (char *) malloc(sizeof(char));
    if(text == NULL){
        fclose(fichier);
        return NULL;
    }
    
    char buffer[TAILLE_TEXT+1];
    buffer[TAILLE_TEXT] = '\0';
    unsigned long int pos = 0;
    int temp;
    char * temp_text;
    
    while(!feof(fichier)){
        temp = fread(buffer,sizeof(char),TAILLE_TEXT,fichier);
        temp_text = (char *)realloc(text,sizeof(char)*(pos+temp));
        if(temp_text == NULL){
            fclose(fichier);
            free(text);
            return NULL;
        }else{text = temp_text;}
        strncpy(&(text[pos]),buffer,temp);
        pos += TAILLE_TEXT;

    }
    fclose(fichier);
    return text;
}

code* trouver_code(char* text){
    int ite=0, taille = 0;
    code** tab = (code**)malloc(0*sizeof(code*));
    int trouve = 0;
    char* echantillon = (char*)malloc(4*sizeof(char));
    echantillon[3]='\0';
    while(text[ite+2]!='\0'){
        echantillon[0]=text[ite];
        echantillon[1]=text[ite+1];
        echantillon[2]=text[ite+2];

        printf("%s\n",echantillon);
        for(int i = 0; i<taille; i++){
            if(strcmp(echantillon, tab[i]->code)==0){
                tab[i]->n += 1;
                trouve = 1;
                printf("je suis passe ici");
            }
        }

        if(trouve == 0){
            taille++;
            tab = (code**)realloc(tab, taille*sizeof(code*));
            tab[taille-1]=(code*)malloc(sizeof(code));
            tab[taille-1]->code = strdup(echantillon);
            tab[taille-1]->n = 1;
        }
        trouve = 0;
        ite++;
    }
    code* res = tab[0];
    int max = tab[0]->n;
    for(int j = 1; j<taille; j++){
        if(tab[j]->n > max){
            max = tab[j]->n;
            res = tab[j];
        }
    }
    printf("taille %d,  ite %d\n", taille, ite);
    return res;
}

int main(){
    char* text = charger_texte("rahaha_le_code_est_la.txt");
    int taille = strlen(text);
    printf("taille texte %d", taille);
    printf("ça passe");
    code* code = trouver_code(text);
    printf("le code est %s avec %d occurences.\n", code->code, code->n);
    return EXIT_SUCCESS;
}