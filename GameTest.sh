#!/bin/bash

counter=1
wc1=0
wc2=0
wc3=0
wc4=0

while [ $counter -le $5 ]
do
    # Ejecutar el juego y redirigir salida
    ./Game $1 $2 $3 $4 -s $RANDOM < dconf > game$counter.res &> gameoutput$counter.txt

    # Obtener las líneas relevantes con las puntuaciones
    scores=$(grep "info: player" gameoutput$counter.txt | awk '{print $3, $6}')
    
    # Determinar al ganador analizando las puntuaciones
    winner=$(echo "$scores" | sort -k2 -n -r | head -1 | awk '{print $1}')

    # Actualizar el contador del ganador
    if [ "$winner" == "$1" ]; then
        ((wc1++))
    elif [ "$winner" == "$2" ]; then
        ((wc2++))
    elif [ "$winner" == "$3" ]; then
        ((wc3++))
    elif [ "$winner" == "$4" ]; then
        ((wc4++))
    fi

    # Imprimir resultados parciales
    echo "$counter: $winner   TOTAL[$1=$wc1, $2=$wc2, $3=$wc3, $4=$wc4]"

    ((counter++))
done

# Imprimir resultados finales
echo "$1 won $wc1"
echo "$2 won $wc2"
echo "$3 won $wc3"
echo "$4 won $wc4"

# Limpiar archivos temporales
rm *.txt *.res
