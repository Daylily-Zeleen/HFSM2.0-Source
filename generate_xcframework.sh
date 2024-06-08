#!/bin/sh

echo $1 $2

scons arch=universal ios_simulator=yes platform=ios target=$1 $2
scons arch=arm64 ios_simulator=no platform=ios target=$1 $2

xcodebuild -create-xcframework -library ./dist/addons/com.daylily_zeleen.hfsm2/bin/libhfsm2.ios.$1.a -library ./dist/addons/com.daylily_zeleen.hfsm2/bin/libhfsm2.ios.$1.simulator.a -output ./dist/addons/com.daylily_zeleen.hfsm2/bin/libhfsm2.ios.$1.xcframework
xcodebuild -create-xcframework -library ./gdextension_dependencies/godot-cpp/bin/libgodot-cpp.ios.$1.arm64.a -library ./gdextension_dependencies/godot-cpp/bin/libgodot-cpp.ios.$1.universal.simulator.a  -output ./dist/addons/com.daylily_zeleen.hfsm2/bin/libgodot-cpp.ios.$1.xcframework
