#!/usr/bin/env bash

in_instance_file=$1
model_dir=$2
out_instance_file=$3
rooms_cap=$4
class_cap=$5

python parse_instance.py $in_instance_file $model_dir $out_instance_file $rooms_cap $class_cap
echo "Remember to change data file in .run to $out_instance_file before doing run_instance" ;