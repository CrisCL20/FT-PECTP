#!/bin/bash

in_instance_file=$1
model_dir=$2
out_instance_file=$3
timeslots_per_day=$4
rooms_cap=$5
class_cap=$6

echo "executing parse_instance.py...";
python parse_instance.py ${in_instance_file} ${model_dir} ${out_instance_file} ${timeslots_per_day} ${rooms_cap} ${class_cap}
echo "Remember to change data file in .run to $out_instance_file before doing run_instance" ;