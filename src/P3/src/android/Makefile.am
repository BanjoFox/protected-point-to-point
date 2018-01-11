## Process this file with automake to produce Makefile.in

CC = /android/ndk/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/arm-eabi-gcc
LD = /android/ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/arm-linux-androideabi/bin/ld

bin_PROGRAMS = \
	p3syssecondary

p3syssecondary_CFLAGS =\
	 -I/android/ndk/platforms/android-5/arch-arm/usr/include\
     -D_p3_SECONDARY=1\
     -Wall\
     -g

p3syssecondary_SOURCES =\
	p3system.c \
    p3sec_net.c \
    p3sec_key_handler.c \
    p3admin.c \
    p3utils.c \
    p3secondary.c

p3syssecondary_LDFLAGS =\
	-L/android/ndk/platforms/android-5/arch-arm/usr/lib \
	-nostdlib\
	 -Wl,--entry=main,-rpath-link=/android/ndk/platforms/android-5/arch-arm/usr/lib,-dynamic-linker=/system/bin/linker \
	-dynamic-linker=/system/bin/linker \
	-lc

