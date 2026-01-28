#!/bin/bash

dirInstances="instances"
dirNSGA="../nsga2-gnuplot-v1.1.6_+Instancia"
dirhv="../hv-1.3-src"

# Máximo de evaluaciones totales
evaluaciones=1000000

# Inicialización de variables
pm=0
pc=0
p=0
instance=""
execution_params=()

# Verificar que se proporcionen argumentos
if [ $# -lt 1 ]; then
    echo "Error: se requiere al menos la instancia como argumento."
    exit 1
fi

# El primer argumento es la instancia

instance=$1
seed=$5
shift 5


# Procesar los argumentos restantes
while [ $# -gt 0 ]; do
    flag="$1"
    
    # Verificar si el argumento actual es un flag (-pm, -pc, -p, -s, etc.)
    case "$flag" in
        -pm)
            if [ $# -gt 1 ]; then
                pm="$2"
                shift 2
            else
                echo "Error: -pm requiere un valor"
                exit 1
            fi
            ;;
        -pc)
            if [ $# -gt 1 ]; then
                pc="$2"
                shift 2
            else
                echo "Error: -pc requiere un valor"
                exit 1
            fi
            ;;
        -p)
            if [ $# -gt 1 ]; then
                p="$2"
                shift 2
            else
                echo "Error: -p requiere un valor"
                exit 1
            fi
            ;;
        *)
            # Si el argumento es numérico o una cadena vacía, lo añadimos a la lista de parámetros de ejecución
            if [[ "$flag" =~ ^[0-9]+(\.[0-9]+)?$ ]] || [ "$flag" = "" ]; then
                execution_params+=("$flag")
                shift
            else
                echo "Unrecognized flag or argument: $flag"
                exit 1
            fi
            ;;
    esac
done

# Calcular mi, número de objetivos y parámetros
mi=$(awk "BEGIN {printf \"%d\",(${evaluaciones}/${p})}")
no=2 # número de objetivos
params="${p} ${mi} ${no} ${pc} ${pm}"
echo "Parámetros: ${params}"

screen=salida
screen2=salida2

# Borrar archivo de salida anterior
rm -rf ${screen}

# Ejecutar NSGA2
echo "./${dirNSGA}/nsga2r 0.${seed} ${dirNSGA}/${dirInstances}/${instance} ${params} > out/${screen}"
./${dirNSGA}/nsga2r 0.${seed} ${dirNSGA}/${dirInstances}/${instance} ${params} > ${screen}

# Buscar óptimo en archivo
exec<"optimos.txt"
# nombreinstancia hv pr1 pr2
while read line; do
    set -- $line
    name=$1
    if [[ ${instance} == ${name} ]]; then
        optimo=$2
        pr1=$3
        pr2=$4
        echo "nombre: ${name}, optimo: ${optimo}, pr1: ${pr1}, pr2: ${pr2}"
    fi
done

# Calcular hv y guardar en quality
echo ${pr1}
echo ${pr2}
factor=2
pr1=$(awk "BEGIN {printf \"%.1f\",${pr1}*${factor}}" | sed 's/,/./')
pr2=$(awk "BEGIN {printf \"%.1f\",${pr2}*${factor}}" | sed 's/,/./')
echo ${pr1}
echo ${pr2}

echo "./${dirhv}/hv -r \"${pr1} ${pr2}\" of.out > ${screen2}"
./${dirhv}/hv -r "${pr1} ${pr2}" of.out > ${screen2}

hv=$(tail -1 ${screen2})
gap=$(awk "BEGIN {printf \"%.2f\",100.00*(${optimo}-${hv})/${optimo}}")
runlength=$(echo ${gap} | sed 's/,/./')

solved="SAT"
runtime=0
best_sol=0

echo "Result for ParamILS: ${solved}, ${runtime}, ${runlength}, ${best_sol}, ${seed}"
