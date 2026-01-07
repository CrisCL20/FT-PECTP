#!/bin/bash

instance=$1
instance_code=$2

cd nsga2-gnuplot-v1.1.6_+Instancia

seeds=("0.01" "0.05" "0.125" "0.232" "0.345" "0.463" "0.587" "0.712" "0.852" "0.999")
p_muts=("0.01" "0.05" "0.4" "0.8" "0.9" "0.95")
popsize="152"
gens="1000"
nobj="2"
pcross="0.95"

for seed in ${seeds[@]}; do
    for p_mut in ${p_muts[@]}; do
        ./nsga2r ${seed} ${instance} ${popsize} ${gens} ${nobj} ${pcross} $p_mut 
        mv best_pop.out ../nsga_results_${instance_code}/best_pop_i${instance}_s${seed}_pmut_${p_mut}.out
        files+=(../nsga_results_${instance_code}/best_pop_i${instance}_s${seed}_pmut_${p_mut}.out)
        
    done
done

