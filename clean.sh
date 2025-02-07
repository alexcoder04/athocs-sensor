#!/bin/sh

sed -E '/SENSITIVE/ { s/"[^"]*"/"CREDENTIALS"/g; s/[[:space:]]*\/\/.*$// }' xiao_esp32c6/xiao_esp32c6.ino >xiao_esp32c6/xiao_esp32c6_nopw.ino

