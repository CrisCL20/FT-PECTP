#!/bin/bash

MODELS_PATH=$1
INSTANCE_PATH=$2
MODE=$3

print_usage() {
    echo "Wrong usage:";
    echo "./run_instance.sh <models_folder> <instance_filepath> <mono|multi>";
}

make_mono() {
    find -maxdepth 1 -type l -delete
    ln -s ../$INSTANCE_PATH/*.dat .
    ln -s ../$MODELS_PATH/*mono.* .
    find -maxdepth 1 -type l -ls
}

make_multi() {
    find -maxdepth 1 -type l -delete
    ln -s ../$INSTANCE_PATH .
    ln -s ../$MODELS_PATH/*multi.* .
    find -maxdepth 1 -type l -ls
}

cd ampl.linux-intel64

case $MODE in
    mono)
        make_mono && ./ampl runnermono.run
        ;;
    multi)
        make_multi && ./ampl runnermulti.run
        ;;
    *) print_usage && exit 0 ;;
esac

find -maxdepth 1 -type l -delete