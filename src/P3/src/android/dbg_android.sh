#!/bin/sh
#
# Using GDB to debug
#
# 1) On the Linux host:
# 
#   adb forward tcp:5039 tcp:5039
# 
# 2) On the Android:
# 
#   cd /data/local/tmp
#   gdbserver :5039 p3secondary
# 
# 3) On the Linux host:
# 
#   arm-eabi-gdb p3secondary
# 
#   In gdb:
# 
#     set solib-search-path /android/ndk/platforms/android-3/arch-arm/usr/lib/
#     shared
#     target remote :5039

echo "In gdb, enter:"
echo ''
echo "set solib-search-path /android/ndk/platforms/android-3/arch-arm/usr/lib/"
echo "shared"
echo "target remote :5039"
echo ''

arm-eabi-gdb p3secondary

