#!/usr/bin/env bash

# embui branch/tag name to fetch
embuirepo='https://github.com/vortigont/EmbUI'
#embuitag="main"
embuitag="v3.2"

#####
# no changes below this point!

optstring=":hfwt:c:"

# etag file
tags=etags.txt
compressor="gz"
compress_cmd="zopfli"
compress_args=""
refresh_data=0
refresh_js=0
refresh_force=0
with_workers=0
embuijs_files='lib.js maker.js dyncss.js'

usage(){
  echo "Usage: `basename $0` [-h] [-t embuitag] [-f] [-c zopfli|gz|br] [-w]"
cat <<EOF
Options:
    -f          force update all resoruces
    -t          git tag to fetch from in embui repo
    -c          compressor to use 'gz' for gzip (default), or 'br' for brotli
    -w          download ace editor syntax highlight resources
    -h          show this help
EOF
}


# parse cmd options
while getopts ${optstring} OPT; do
    case "$OPT" in
        c)
            echo "Set compressor to: $OPTARG"
            compress_cmd=$OPTARG
            ;;
        f)
            echo "Force refresh"
            rm -f $tags
            refresh_data=1
            refresh_js=1
            refresh_force=1
            rm -f $tags
            ;;
        h)
            usage
            exit 0
            ;;
        t)
            echo "EmbUI tag is set to: $OPTARG"
            embuitag=$OPTARG
            ;;
        w)
            with_workers=1
            ;;
        :)
            echo "$0: mandatory argument is missin for -$OPTARG" >&2
            ;;
        \?)
            # getopts issues an error message
            usage >&2
            exit 1
            ;;
    esac
done

compress_zopfli(){
    local src="$1"
    zopfli ${compress_args} ${src}
    rm -f ${src}
}

compress_gz(){
    local src="$1"
    gzip ${compress_args} ${src}
}

compress_br(){
    local src="$1"
    brotli ${compress_args} ${src}
}


if [[ "$compress_cmd" = "gz" ]] ; then
    compress_cmd=`which gzip`
    if [ "x$compress_cmd" = "x" ]; then
        echo "ERROR: gzip compressor not found!"
        exit 1
    fi
    compress_cmd=compress_gz
    compress_args="-9"
elif [[ "$compress_cmd" = "zopfli" ]] ; then
    compress_cmd=`which zopfli`
    if [ "x$compress_cmd" = "x" ]; then
        echo "ERROR: zopfli compressor not found!"
        exit 1
    fi
    compress_cmd=compress_zopfli
elif [[ "$compress_cmd" = "br" ]] ; then
    compress_cmd=`which brotli`
    if [ "x$compress_cmd" = "x" ]; then
        echo "ERROR: brotli compressor not found!"
        exit 1
    fi
    compress_cmd=compress_br
    compress_args="--best"
fi
echo "Using compressor: $compress_cmd"

[ -f $tags ] || touch $tags

# check github file for a new hash
freshtag(){
    local url="$1"
    echo "run $url"
    etag=$(curl -sL -I $url | grep etag | awk '{print $2}')
    if [[ "$etag" = "" ]] ; then
	    return 0
    fi
    echo "$url $etag" >> newetags.txt
    if [ $(grep -cs $etag $tags) -eq 0 ] ; then
        #echo "new tag found for $url"
        return 0
    fi
    #echo "old tag for $url"
    return 1
}

# get resource file from EmbUI repo
getRes(){
    local res=$1
    local url="${embuirepo}/raw/$embuitag/resources/html/${res}"
    if freshtag ${url} ; then
        curl -sL $url  > ../data/${res}
        ${compress_cmd} ../data/${res}
    fi
}

# update local file if source has newer version
updlocalarchive(){
    local res=$1
    echo "check: $res"
    [ ! -f html/${res} ] && return
    if [ ! -f ../data/${res}.${compressor} ] || [ html/${res} -nt ../data/${res}.${compressor} ] ; then
        cp html/${res} ../data/${res}
        ${compress_cmd}  ../data/${res} && touch -r html/${res} ../data/${res}.${compressor}
    fi
}


echo "Preparing resources for FireLamp FS image into ../data/ dir" 

mkdir -p ../data/css ../data/js ../data/extras ../data/login

# собираем скрипты и стили из репозитория EmbUI
if freshtag ${embuirepo}/raw/$embuitag/resources/data.zip ; then
    refresh_data=1
    echo "EmbUI resources requires updating"
else
    echo "EmbUI is up to date"
fi

# check for newer js files
for f in ${embuijs_files}
do
    #echo "check $f"
    if freshtag ${embuirepo}/raw/$embuitag/resources/html/js/${f} ; then
        refresh_js=1
        #break
    fi
done

# if any of the styles needs updating, than we need to repack both embui and local files
if [ $refresh_data -eq 1 ] ; then

    echo "Refreshing EmbUI css files/pics..."

    curl -sL ${embuirepo}/raw/$embuitag/resources/data.zip > embui.zip
    unzip -o -d ../data/ embui.zip "css/*" "js/*"

    # append our local styles to the embui
    for f in html/css/style_*
    do
        #echo "add css: $f"
        # decompress embui's file (all in gz)
        #${compress_cmd} -d ../data/css/$( basename $f).${compressor}
        gzip -df ../data/css/$( basename $f).gz
        # append our file
        cat $f >> ../data/css/$( basename $f)
        ${compress_cmd} ../data/css/$( basename $f)
        touch -r $f ../data/css/$( basename $f).${compressor}
        rm -f ../data/css/$( basename $f)
    done

    rm -f embui.zip
fi

# update EmbUI js files
if [ $refresh_js -eq 1 ] ; then
    echo "refreshing embui js files..."

    rm -f embui.js
    rm -f ../data/js/embui.js*
    for f in ${embuijs_files}
    do
        curl -sL ${embuirepo}/raw/$embuitag/resources/html/js/${f} >> embui.js
        #echo "fetch $f"
    done
    ${compress_cmd} embui.js && mv embui.js.${compressor} ../data/js/ && rm -f embui.js

    curl -sL ${embuirepo}/raw/$embuitag/resources/html/js/ui_sys.json > ../data/js/ui_sys.json
    ${compress_cmd} ../data/js/ui_sys.json
fi

echo "Update local resources"
lamp_files='index.html favicon.ico css/wp_dark.svg css/wp_light.svg js/ui_lamp.json'
for f in ${lamp_files}
do
    updlocalarchive $f
done

# check if there are any fresh data for the project to merge with EmbUI
#for f in html/css/*.svg
#do
#    echo "style: $f"
#    src="css/$( basename $f)"
#    updlocalarchive $src
#done

# обновить файлы в /login/
#for f in html/login/*
#do
#    updlocalarchive login/$( basename $f)
#done

# обновляем скрипты/стили специфичные для лампы
[ ! -f ../data/css/lamp.css.${compressor} ] || [ html/css/custom_drawing.css -nt ../data/css/lamp.css.${compressor} ] && cat html/css/custom_drawing.css | ${compress_cmd} ${compress_args} > ../data/css/lamp.css.${compressor}
# update lamp's js scripts
cat html/js/*.js > ../data/js/lamp.js && ${compress_cmd} ../data/js/lamp.js


# update plain files
#cp -u html/.exclude.files ../data/
cp -u html/manifest.webmanifest ../data/
cp -u html/css/*.webp ../data/css/

#cp -u html/events_config.json ../data/
#cp -u html/buttons_config.json ../data/

: '
###
# обновляем ace-editor в data/extras folder
# collect all required ACE editor extensions here
acew_url='https://github.com/ajaxorg/ace-builds/raw/master/src-min-noconflict'
# syntax highlight (if requred)
ace_workers="worker-html.js worker-css.js worker-javascript.js worker-json.js"
ace_dst="../data/extras"
if [ $refresh_force -eq 1 ] ||
   [ $(freshtag $acew_url/ace.js) ] || \
   [ $(freshtag $acew_url/mode-html.js) ] || \
   [ $(freshtag $acew_url/mode-json.js) ] ; then

        echo "Updating AceEditor resources"
        curl -sL "$acew_url/ace.js" > $ace_dst/acefull.js
        curl -sL "$acew_url/mode-html.js" >> $ace_dst/acefull.js
        curl -sL "$acew_url/mode-json.js" >> $ace_dst/acefull.js
        ${compress_cmd} ${compress_args} -f $ace_dst/acefull.js
        rm -f $ace_dst/acefull.js
else
    echo "AceEditor is up to date"
fi

if [ $with_workers -eq 1 ] ; then
    for f in ${ace_workers}
    do
        if [ $refresh_force -eq 1 ] || [ $(freshtag ${acew_url}/${f}) ] ; then
            curl -sL ${acew_url}/${f} | ${compress_cmd} ${compress_args} > $ace_dst/${f}.${compressor}
        fi
    done
else
    rm -f $ace_dst/worker-*
fi

if freshtag https://github.com/ajaxorg/ace/raw/master/doc/site/images/favicon.ico ; then
    curl -sL https://github.com/ajaxorg/ace/raw/master/doc/site/images/favicon.ico | ${compress_cmd} ${compress_args} > $ace_dst/ace.ico.${compressor}
fi

#updlocalarchive extras/edit.htm
'

mv -f newetags.txt $tags

#cd ./data
#zip --filesync -r -0 --move --quiet ../data.zip .
#cd ..
#rm -r ./data
#echo "Content of data.zip file should be used to create LittleFS image"
