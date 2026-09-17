#!/bin/bash

cd nsga2v3.1
mkdir -p logs
workers=4

INSTANCE_FILEPATH=$1

seeds=("0.01" "0.05" "0.125" "0.232" "0.345" "0.463" "0.587" "0.712" "0.852" "0.999")
nobj=2
popsize=500
gens=10000
pc=0.9
pm=0.2
pm_ts_swap=0.6
pm_schedule=0.6
top_tslots=5

parallel -j ${workers} --bar \
    "./nsga2r {1} {2} ${popsize} ${gens} ${nobj} ${pc} ${pm} ${pm_ts_swap} ${pm_schedule} ${top_tslots} > logs/out_\$(basename {2} .dat)_s{1}.log 2>&1" \
    ::: "${seeds[@]}" \
    ::: "${INSTANCE_FILEPATH}"
