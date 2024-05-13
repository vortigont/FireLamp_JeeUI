# From: https://github.com/letscontrolit/ESPEasy/blob/mega/tools/pio/post_esp32.py
#
# Combines separate bin files with their respective offsets into a single file
# This single file must then be flashed to an ESP32 node with 0 offset.
#
# Original implementation: Bartłomiej Zimoń (@uzi18)
# Maintainer: Gijs Noorlander (@TD-er)
#
# Special thanks to @Jason2866 (Tasmota) for helping debug flashing to >4MB flash
# Thanks @jesserockz (esphome) for adapting to use esptool.py with merge_bin
#
# Typical layout of the generated file:
#    Offset | File
# -  0x1000 | ~\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dout_40m.bin
# -  0x8000 | ~\ESPEasy\.pio\build\<env name>\partitions.bin
# -  0xe000 | ~\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin
# - 0x10000 | ~\ESPEasy\.pio\build\<env name>/<built binary>.bin

# cli image builder
# https://github.com/ThingPulse/esp32-epulse-feather-testbed/tree/main

Import("env")

env = DefaultEnvironment()
platform = env.PioPlatform()

import os
import sys
import csv
import subprocess
from os.path import join
from genericpath import exists
from colorama import Fore, Back, Style

sys.path.append(join(platform.get_package_dir("tool-esptoolpy")))
import esptool

chip = env.get("BOARD_MCU")
sections = env.subst(env.get("FLASH_EXTRA_IMAGES"))

def esp32_detect_flashsize():
    uploader = env.subst("$UPLOADER")
    if not "upload" in COMMAND_LINE_TARGETS:
        return "4MB",False
    if not "esptool" in uploader:
        return "4MB",False
    else:
        esptoolpy = join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py")
        esptoolpy_flags = ["flash_id"]
        esptoolpy_cmd = [env["PYTHONEXE"], esptoolpy] + esptoolpy_flags
        try:
            output = subprocess.run(esptoolpy_cmd, capture_output=True).stdout.splitlines()
            for l in output:
                if l.decode().startswith("Detected flash size: "):
                    size = (l.decode().split(": ")[1])
                    print("Did get flash size:", size)
                    stored_flash_size_mb = env.BoardConfig().get("upload.flash_size")
                    stored_flash_size = int(stored_flash_size_mb.split("MB")[0]) * 0x100000
                    detected_flash_size = int(size.split("MB")[0]) * 0x100000
                    if detected_flash_size > stored_flash_size:
                        env.BoardConfig().update("upload.flash_size", size)
                        return size, True
            return "4MB",False
        except subprocess.CalledProcessError as exc:
            print(Fore.YELLOW + "Did get chip info failed with " + str(exc))
            return "4MB",False

flash_size_from_esp, flash_size_was_overridden = esp32_detect_flashsize()


def esp32_build_filesystem(fs_size):
    #files = env.GetProjectOption("custom_files_upload").splitlines()
    filesystem_dir = env.subst("$PROJECT_DATA_DIR")
    #if not os.path.exists(filesystem_dir):
    #    os.makedirs(filesystem_dir)
    print("Creating %dKiB filesystem with content:" % (int(fs_size, 0)/1024) )
    if not os.listdir(filesystem_dir):
        print("No files added -> will NOT create littlefs.bin and NOT overwrite fs partition!")
        return False
    # this does not work on GitHub, results in 'mklittlefs: No such file or directory'
    tool =  env.subst(env["MKFSTOOL"])
    #tool = "~/.platformio/packages/tool-mklittlefs/mklittlefs"
    cmd = (tool,"-c",filesystem_dir,"-s",fs_size,join(env.subst("$BUILD_DIR"),"littlefs.bin"))
    returncode = subprocess.call(cmd, shell=False)
    # print(returncode)
    return True

def esp32_create_combined_bin(source, target, env):
    print("Generating combined binary for serial flashing")
    # The offset from begin of the file where the app0 partition starts
    # This is defined in the partition .csv file
    # factory_offset = -1      # error code value - currently unused
    app_offset = 0x10000     # default value for "old" scheme
    fs_offset = -1           # error code value

    with open(env.BoardConfig().get("build.partitions")) as csv_file:
        print("Read partitions from ",env.BoardConfig().get("build.partitions"))
        csv_reader = csv.reader(csv_file, delimiter=',')
        line_count = 0
        for row in csv_reader:
            if line_count == 0:
                print(f'{",  ".join(row)}')
                line_count += 1
            else:
                if (len(row) < 4):
                    continue
                print(f'{row[0]}   {row[1]}   {row[2]}   {row[3]}   {row[4]}')
                line_count += 1
                if(row[0] == 'app0'):
                    app_offset = int(row[3],base=16)
                # elif(row[0] == 'factory'):
                #     factory_offset = int(row[3],base=16)
                elif(row[0] == 'spiffs'):
                    partition_size = row[4]
                    if esp32_build_filesystem(partition_size):
                        fs_offset = int(row[3],base=16)


    new_file_name = env.subst("$BUILD_DIR/factory.${PIOENV}.bin")
    print("Factory image file: " + new_file_name)
    firmware_name = env.subst("$BUILD_DIR/${PROGNAME}.bin")
    #tasmota_platform = esp32_create_chip_string(chip)

    flash_size = env.BoardConfig().get("upload.flash_size", "4MB")
    flash_mode = env["__get_board_flash_mode"](env)
    flash_freq = env["__get_board_f_flash"](env)

    cmd = [
        "--chip",
        chip,
        "merge_bin",
        "-o",
        new_file_name,
        "--flash_mode",
        flash_mode,
        "--flash_freq",
        flash_freq,
        "--flash_size",
        flash_size,
    ]

    print("    Offset | File")
    for section in sections:
        sect_adr, sect_file = section.split(" ", 1)
        print(f" -  {sect_adr} | {sect_file}")
        cmd += [sect_adr, sect_file]

    # "main" firmware to app0 - mandatory
    print(f" - {hex(app_offset)} | {firmware_name}")
    cmd += [hex(app_offset), firmware_name]


    upload_protocol = env.subst("$UPLOAD_PROTOCOL")
    if(upload_protocol == "esptool") and (fs_offset != -1):
        fs_bin = join(env.subst("$BUILD_DIR"),"littlefs.bin")
        if exists(fs_bin):
            before_reset = env.BoardConfig().get("upload.before_reset", "default_reset")
            after_reset = env.BoardConfig().get("upload.after_reset", "hard_reset")
            print(f" - {hex(fs_offset)}| {fs_bin}")
            cmd += [hex(fs_offset), fs_bin]
            env.Replace(
            UPLOADERFLAGS=[
            "--chip", chip,
            "--port", '"$UPLOAD_PORT"',
            "--baud", "$UPLOAD_SPEED",
            "--before", before_reset,
            "--after", after_reset,
            "write_flash", "-z",
            "--flash_mode", "${__get_board_flash_mode(__env__)}",
            "--flash_freq", "${__get_board_f_flash(__env__)}",
            "--flash_size", flash_size
            ],
            UPLOADCMD='"$PYTHONEXE" "$UPLOADER" $UPLOADERFLAGS ' + " ".join(cmd[7:])
            )
            print("Will use custom upload command for flashing operation to add file system defined for this build target.")

    print('Using esptool.py arguments: %s' % ' '.join(cmd))
    esptool.main(cmd)
    # rename firmware image before uploading to artifactory
    os.rename(firmware_name, env.subst("$BUILD_DIR/${PROGNAME}.${PIOENV}.bin"))

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp32_create_combined_bin)
