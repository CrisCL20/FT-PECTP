#!/usr/bin/env bash

models_path=$1
instance_file=$2

print_usage() {
    echo "Wrong usage:";
    echo "./run_instance.sh <models_folder> <instance_filepath> <mono|multi>";
}

make_mono() {
    find -maxdepth 1 -type l -delete
    ln -s ../$models_path/$instance_file .
    ln -s ../$models_path/*mono.* .
    find -maxdepth 1 -type l -ls
}

make_multi() {
    find -maxdepth 1 -type l -delete
    ln -s ../$models_path/$instance_file .
    ln -s ../$models_path/*multi.* .
}

cd ampl.linux-intel64

case $3 in
    mono)
        make_mono && ./ampl runnermono.run
        ;;
    multi)
        make_multi && ./ampl runnermulti.run
        ;;
    *) print_usage && exit 0 ;;
esac

find -maxdepth 1 -type l -delete