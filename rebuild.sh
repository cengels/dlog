#!/bin/zsh

rm -rf out/
mkdir out/
cd out/
cmake $@ ../
cmake --build .