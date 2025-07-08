#!/usr/bin/env bash

for file in nsga_results_big/*; do
    awk '! /^#/ && $1 != "Total" {print $1 " " $2;}' $file
done