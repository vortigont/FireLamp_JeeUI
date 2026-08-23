#!/bin/env python

#from subprocess import check_output, CalledProcessError
import platform
import os.path
import subprocess
Import("env")

from datetime import datetime
try:
    from zopfli.gzip import compress
    print("found Zopfli compressor, good!")
except:
    from gzip import compress
    print("Using gzip compressor, try to install python zopfli module for better compression")

#print("************** Current CLI targets", COMMAND_LINE_TARGETS)
#print("************** Current Build targets", BUILD_TARGETS)

outdir='embed/'
jsdir = 'resources/html/js/'
cssdir = 'resources/html/css/'
in_gz_files = ["resources/html/index.html", jsdir + "ui.json", jsdir + "ui.i18n.json", jsdir + "informer.js",
    cssdir + "style.css", cssdir + "style_dark.css", cssdir + "style_light.css",
]
#combine_js_files = [jsdir + "firelamp.js", jsdir + "drawing.js"]
static_files = [
    # libs
    jsdir + "lodash.js.gz", cssdir + "pure.css.gz",
    # embui
    jsdir + "embui.js.gz",  jsdir + "ui_embui.json.gz", jsdir + "ui_embui.i18n.json.gz",
    jsdir + "ui_embui.lang.json.gz", jsdir + "tz.json.gz",
    # styles and images
    cssdir + "menu.jpg", cssdir + "menu-dark.webp", cssdir + "menu-light.webp",
    cssdir + "wp_dark.svg.gz", cssdir + "wp_light.svg.gz", 
    cssdir + "android-chrome-192x192.webp", cssdir + "android-chrome-512x512.webp"
]

def set_compressor():
    cmd = "where" if platform.system() == "Windows" else "which"
    try:
        subprocess.check_output([cmd, 'zopfli'])
        compressor='zopfli'
        print("found external Zopfli compressor, good!")
    except:
        compressor='internal'

def print_hdr_data(filepath, f_hndlr):
    flat_name = filepath
    #flat_name = outdir + os.path.basename(filepath)
    # replace special characters with underscore
    chars = ".#-/"
    for c in chars:
        flat_name = flat_name.replace(c, "_")
    #f_hndlr.write("#define BLOB_LEN_%s %d\n" % (flat_name, len(gz)))
    # shorten symbol name
    short_name = flat_name.replace("resources_html_js", "jsdir")
    short_name = short_name.replace("resources_html_css", "cssdir")
    f_hndlr.write("extern const uint8_t %s_start[] asm(\"_binary_%s_start\");\n" % (short_name, flat_name))
    f_hndlr.write("extern const uint8_t %s_end[] asm(\"_binary_%s_end\");\n" % (short_name, flat_name))

def compress_data(input, dst_file):
    try:
        with open(dst_file, 'wb') as f_out:
            f_out.write(compress(input))
    except:
        print("error writing %s" % dst_file)


def compress_file(src_file, f_hndlr):
    try:
        #basename = os.path.basename(src_file)
        #subprocess.check_output(["zopfli", src, "> "],  stdout=outdir + basename + '.gz')
        with open(src_file, 'rb') as f_in:
            print("*** Compressing %s file..." % src_file)
            fpath = outdir + os.path.basename(src_file) + '.gz'
            compress_data(f_in.read(), fpath)
            print_hdr_data(fpath, f_hndlr)
    except:
        print("error compressing %s" % src_file)


def embed_resources():
    #if not os.path.exists(outdir):
    #    os.makedirs(outdir)
    # make header file
    with open(outdir + 'embed.h', 'wt') as f_hdr:
        f_hdr.write("static const char __BUILD_TSTAMP[] = \"%d\";\n" % int(datetime.now().timestamp()))
        print("*** Compressing embedded resources... ***")

        # compress individual files
        for f in in_gz_files:
            compress_file(f, f_hdr)

        # combine js scripts
        #jsall = ''.join([open(f, 'r', encoding='utf-8').read() for f in combine_js_files])
        #fpath = outdir + 'informer.js.gz'
        #compress_data(jsall, fpath)
        #print_hdr_data(fpath, f_hdr)

        # make header data for static files
        for f in static_files:
            print_hdr_data(f, f_hdr)



#env.AddPreAction("${BUILD_DIR}/src/http.cpp.o", testme)
#env.AddPreAction("buildprog", testme)

embed_resources()
