#!/bin/bash

instance_code=$1

for file in nsga_results_${instance_code}/*; do
    awk '! /^#/ && $1 != "Total" {print "[" $1 ", " $2 "],";}' $file
done