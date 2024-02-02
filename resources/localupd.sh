#!/usr/bin/env bash

compressor="gz"
compress_cmd=""
compress_args=""

updlocalarchive(){
    local res=$1
    echo "check: $res"
    [ ! -f html/${res} ] && return
    [ ! -f ../data/${res}.${compressor} ] || [ html/${res} -nt ../data/${res}.${compressor} ] && ${compress_cmd} ${compress_args} -c html/${res} > ../data/${res}.${compressor} && touch -r html/${res} ../data/${res}.${compressor}
}

compress_cmd=`which gzip`
if [ "x$compress_cmd" = "x" ]; then
    echo "ERROR: gzip compressor not found!"
    exit 1
fi
compress_args="-9"



echo "Update local resources"
lamp_files='index.html js/ui_lamp.json favicon.ico css/wp_dark.svg css/wp_light.svg'
for f in ${lamp_files}
do
    updlocalarchive $f
done
