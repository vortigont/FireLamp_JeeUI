#!/usr/bin/python

import os
import glob
import shutil
import subprocess

Import("env")

env = DefaultEnvironment()
platform = env.PioPlatform()

def before_buildfs(source, target, env):
    print("before_buildfs")

def add_huwf2_configs(source, target, env):
    print("Adding HU-WF2 configuration to the FS image")
    for filename in glob.glob(os.path.join(env.subst("$PROJECT_DIR") + "/profiles/hu-wf2", '*.json')):
        print(filename)
        shutil.copy(filename, env.subst("$PROJECT_DATA_DIR"))

def remove_huwf2_configs(source, target, env):
    print("Removing HU-WF2 configuration files")
    for filename in os.listdir(os.path.join(env.subst("$PROJECT_DIR"), "profiles/hu-wf2")):
        if filename.endswith('.json'):
            print(env.subst("$PROJECT_DATA_DIR") + "/" + filename)
            os.remove(env.subst("$PROJECT_DATA_DIR") + "/" + filename)
    subprocess.run(["git", "reset", "--hard", "HEAD"])

env.AddPreAction("$BUILD_DIR/littlefs.bin", add_huwf2_configs)
env.AddPostAction("$BUILD_DIR/littlefs.bin", remove_huwf2_configs)
