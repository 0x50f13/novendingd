target_build(){
  require_command gcc
  require_directory bin
  change_dir src
  exec "gcc -O3 -Wall -Wpedantic -llog *.c -o../bin/novendingd"
  leave_dir
  success "Succesfully built novendingd."
}

target_install(){
  require_command adb
  exec adb root  
  exec adb shell mount -o remount,rw /
  exec adb shell rm -rf /system/xbin/novendingd
  exec adb shell rm -rf /system/etc/init/novendingd.rc
  exec adb shell rm -rf /system/addon.d/50-novendingd.sh
  exec adb push bin/novendingd /system/xbin
  exec adb push novendingd.rc /system/etc/init/
  exec adb push 50-novendingd.sh /system/addon.d/
  exec adb shell chmod 755 /system/addon.d/50-novendingd.sh
  success "Succesfully installed novedningd to the device."
}

target_all(){
  target_build
  target_install
}
