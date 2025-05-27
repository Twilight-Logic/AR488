
# to upload on specific serial port:
# MONITOR_PORT=/dev/ttyACM3 make upload

# TODO target to update version FWVER??

# .PHONY: test
# test: *.h *.cpp
# 	@rm -f test_bin
# 	@gcc -o test_bin test.cpp -L /usr/lib64 -lstdc++
# 	@./test_bin

.PHONY: findserial
findserial:
	@./findserial.sh

# comment this out to print arduinoMakefile diagnostics
ARDUINO_QUIET=1

ARDUINO_DIR=$(HOME)/.arduino-1.8.13/
ARDMK_DIR=$(HOME)/Arduino/arduinoMakefile

BOARD_TAG    = uno
include $(ARDMK_DIR)/Arduino.mk
