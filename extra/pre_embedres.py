#!/bin/env python

Import("env")

print("************** Current CLI targets", COMMAND_LINE_TARGETS)
print("************** Current Build targets", BUILD_TARGETS)


def testme(source, target, env):
    print("************************************************************************* Compressing %s file..." % "some")

env.AddPreAction("${BUILD_DIR}/src/http.cpp.o", testme)
