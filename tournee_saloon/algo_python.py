#dans map.csv trouvé le saloon qui apparait le plus de fois
#parcours de l'ensemble des lignes
import csv as csv
import random as rnd

f = open("pokemons.csv","r")
tab = csv.reader(f,delimiter=",")

for i in tab:
    œ