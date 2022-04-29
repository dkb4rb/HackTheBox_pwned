.PHONY: all clean

all: sandbox

jails:
	mkdir jails; chmod 0771 jails


sandbox: jails sandbox.c copy.c tracing.c
	gcc sandbox.c copy.c tracing.c -static -o sandbox
	sudo setcap 'cap_setpcap,cap_sys_admin,cap_setuid,cap_setgid,cap_sys_chroot=+eip' ./sandbox

clean:
	for i in $(shell find jails -maxdepth 2 -name proc); do sudo umount $$i; done
	rm -rf sandbox jails/*
