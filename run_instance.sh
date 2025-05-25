#!/usr/bin/env bash

models_path=$1
instance_file=$2

cd ampl.linux-intel64

find -maxdepth 1 -type l -delete

ln -s ../$models_path/$instance_file .

case $3 in
    mono)
        ln -s ../$models_path/*mono.* .
        ./ampl runnermono.run
        ;;
    multi)
        ln -s ../$models_path/*multi.* .
        ./ampl runnermulti.run
        ;;
    *) echo "wrong usage" && exit 0 ;;
esac

find -maxdepth 1 -type l -delete