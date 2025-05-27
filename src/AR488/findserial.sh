#!/bin/sh -e

# A simple script to find serial ports and print some info, assuming they are usb.
#
# Note that it will print some errors if there are no such devices present.

if [[ $(uname -s) != "Linux" ]] then
  echo "sorry, this only works on linux"
  exit 1
fi

for t in /sys/class/tty/tty[AU]*; do
  d=$t/device/../..
  name=$(basename $t)
  pid=$(cat $d/idProduct)
  vid=$(cat $d/idVendor)
  mfg=$(cat $d/manufacturer)
  prod=$(cat $d/product)
  echo "$(tput setaf 2)$name$(tput sgr0) pid=$pid vid=$vid mfg=$mfg prod=$prod"
done
echo -e "now, replace the X's and run\n  $(tput setaf 3)MONITOR_PORT=/dev/ttyXXX make upload $(tput sgr0)"
