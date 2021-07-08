# NoVending
NoVending is a small daemon that prevents Google Play to run in main profile
for too long. 
## Idea
Suppose you want to have your work profile with Google Play and its' services
and main profile without it. So you can just disable it in main profile and
everything would work. Ok, almost evverything. Each time you will try to 
install app in work profile the receiver of app state changes in main profile
would be triggered even if Google Play is disabled. And so this would start
it in your main profile. Moreover if you now turn off your work profile
a Google Play will continue run in main profile despite that fact it is
disabled. So this program is aimed to kill Google Play in main profile 
whenever it is starts there.
## Program logic
The program kills Google Play in main profile immeadiatly if there is no
Google Play process in work profile. Otherwise it allows it to run for some
time(by default 5 seconds) it to complete some priviliged actions. (Without
the second part Google Play fails to install apps in work profile).
## Configuring
Before building a program you should find username of Google Play package
in work profile and main profile. The way for LineageOS 17.1 is as follows:
1. Enable USB debugging and USB debugging as root
2. Turn on work profile(and open Google Play)
3. Connect your phone to a computer with platform-tools installed and run:
```
$ adb root
* daemon not running; starting now at tcp:5037
* daemon started successfully
restarting adbd as root
$ adb shell
# ps -A | grep com.android.vending
u10_a144     15987   678 1330148  86904 SyS_epoll_wait      0 S com.android.vending
u10_a144     16210   678 1251356  59204 SyS_epoll_wait      0 S com.android.vending:download_service
u0_a144      16453   678 1272840  69020 SyS_epoll_wait      0 S com.android.vending
u10_a144     16705   678 1220612  45624 SyS_epoll_wait      0 S com.android.vending:instant_app_installer
# id u10_a144 # <-- That is work profile username
uid=1010144(u10_a144) gid=1010144(u10_a144) groups=1010144(u10_a144), context=u:r:su:s0
# id u0_a144 # <-- That is main profile username
uid=10144(u0_a144) gid=10144(u0_a144) groups=10144(u0_a144), context=u:r:su:s0
```
So userid of google play in main profile is 10144 and in work profile 1010144. Now on your computer
open `src/config.h` and replace number after `#define UID_MAIN` with userid in main profile and
after `#define UID_ISOLATED` with userid in work profile.
## Building and installing
For automated build and install you would need an Android NDK and [this toolchain](https://github.com/sjitech/android-gcc-toolchain).
Building steps are as follows:
* Run toolchain: `bash /path/to/android-gcc-toolchain`
* Enter to project directory `cd /path/to/novendingd`
* With your device connected and USB debugging allowed run `./build.sh all`
This would build and install `novendingd` to your device. If everything works correctly after rebooting your device you should see 
following message in logcat:
```
07-08 13:45:47.574  3442  3442 I novendingd: daemon starting...
```
The program was tested with LineageOS 17.1(twolip) and NikGApps core.
