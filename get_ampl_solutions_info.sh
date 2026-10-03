#!/bin/bash

instance=$1

nadir_points=$(awk '/MVF/,/;/ { if ($1 ~ /^[0-9]+$/) print $3 }' ${instance})
sols=$(awk '/FN1/,/;/ { 
    if ($1 ~ /^[0-9]+$/) { 
	            rows = (rows ? rows ",\n" : "\n") "[" $4 "," $5 "]" 
		        } 
		} 
		END { print "[" rows "]" }' ${instance})
sum_time=$(awk '/FN1/,/;/ { if ($1 ~ /^[0-9]+$/) suma += $6 } END { print suma }' ${instance})

echo -e "Puntos de nadir:\n${nadir_points}\n" 
echo -e "Lista de soluciones:\n${sols}\n"
echo "Tiempo total de ejecución: ${sum_time}"
