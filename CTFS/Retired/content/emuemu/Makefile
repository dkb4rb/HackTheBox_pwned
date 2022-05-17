CC := gcc
CFLAGS := -std=c99 -Wall -Werror -Wextra -Wpedantic -Wconversion -Wsign-conversion

SOURCES := $(wildcard *.c)
TARGETS := $(SOURCES:.c=)

.PHONY: install clean

install: $(TARGETS)
	@echo "[+] Installing program files"
	install --mode 0755 emuemu /usr/bin/
	mkdir --parent --mode 0755 /usr/lib/emuemu /usr/lib/binfmt.d
	install --mode 0750 --group dkb4rb reg_helper /usr/lib/emuemu/
	setcap cap_dac_override=ep /usr/lib/emuemu/reg_helper

	@echo "[+] Register OSTRICH ROMs for execution with EMUEMU"
	echo ':EMUEMU:M::\x13\x37OSTRICH\x00ROM\x00::/usr/bin/emuemu:' \
		| tee /usr/lib/binfmt.d/emuemu.conf \
		| /usr/lib/emuemu/reg_helper

clean:
	rm -f -- $(TARGETS)
