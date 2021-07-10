#include "config.h"
#include "aassert.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <errno.h>
#include <android/log.h>


void kill_by_user(uid_t uid){
    struct dirent* _dirent;
    DIR* dir;

    dir = opendir ("/proc");
    if (dir == NULL) {
        cerror("opendir");
    }

    while ((_dirent = readdir(dir)) != NULL) {
        if(!_dirent){
           cerror("readdir");
        }

        struct stat info;
        char* name = (char*)malloc((7+strlen(_dirent->d_name))*sizeof(char));
        strcpy(name, "/proc/");
        strcat(name, _dirent->d_name);

        if(stat(name,&info)==-1){
           if(errno == 2){ //This means that process has died while we were iterating
              __android_log_print(ANDROID_LOG_WARN, MODNAME, "attempted to stat %s, but it has disappeared!", name);
              free(name);
              continue;
           }else{ //SELinux? Kernel? Mad implementation of procfs?
              cerror("stat");
           }
        }
        if(info.st_uid == uid){
          kill(atoi(_dirent->d_name), SIGKILL);
#if DEBUG
          __android_log_print(ANDROID_LOG_DEBUG, MODNAME, "Killing %s",_dirent->d_name);
#endif        
        }
        free(name);
    }
    closedir(dir);
}

uint64_t get_proc_count(uid_t uid){
    uint64_t count = 0;
    struct dirent* _dirent;
    DIR *dir;

    dir = opendir ("/proc");
    if (dir == NULL) {
        cerror("opendir");     
    }

    while ((_dirent = readdir(dir)) != NULL) {        
        if(!_dirent){
           cerror("readdir");
        }
        
        struct stat info;
        char* name = (char*)malloc((7+strlen(_dirent->d_name))*sizeof(char));
        strcpy(name, "/proc/");
        strcat(name, _dirent->d_name);

        if(stat(name,&info)==-1){
          if(errno == 2){
            __android_log_print(ANDROID_LOG_WARN, MODNAME, "attempted to stat %s, but it has disappeared!", name);
            free(name);
            continue;
          }else{
            cerror("stat");
          }
        }
        if(info.st_uid == uid){
          ++count;
        }
        free(name);
    }
    closedir(dir);
    return count;
}

int main(){
    errno = 0;
    aassert(!errno);
    __android_log_print(ANDROID_LOG_INFO, MODNAME, "daemon starting...");

#if DAEMONIZE    
    if(daemon(0,0)){
       __android_log_print(ANDROID_LOG_ERROR, MODNAME, "failed to daemonize: %s(%d)",strerror(errno),errno);
       exit(-1);
    }
#endif

    aassert(getuid()==0);

    uint64_t idle_remain = FULL_IDLE_TIME;

    bool wasRunning = false;
    for(;;){
       bool isRunningIsolated = get_proc_count(UID_ISOLATED);
       bool isRunningMain = get_proc_count(UID_MAIN);
#if DEBUG       
       if(isRunningIsolated){
          __android_log_print(ANDROID_LOG_DEBUG, MODNAME, "Detected isolated run of vending");
       }
       if(isRunningMain){
          __android_log_print(ANDROID_LOG_DEBUG, MODNAME, "Detected run of main vending process");
       }
#endif
       if(isRunningMain&&!wasRunning){
          wasRunning = true;
          __android_log_print(ANDROID_LOG_INFO, MODNAME, "vending has started up in main profile");
          idle_remain = FULL_IDLE_TIME;
       }
       if(!isRunningIsolated&&isRunningMain){
          __android_log_print(ANDROID_LOG_INFO, MODNAME, "vending is not running: killing main profile instances");
          kill_by_user(UID_MAIN);
       }
       if(isRunningIsolated&&isRunningMain){
          --idle_remain;
          if(idle_remain==0){
             __android_log_print(ANDROID_LOG_INFO, MODNAME, "vending has exceeded its runtime in main profile");
             kill_by_user(UID_MAIN);
             wasRunning=false;
          }
       }
#if NO_GSERVICES
       if(get_proc_count(UID_GSERVICES)>0){
          __android_log_print(ANDROID_LOG_INFO, MODNAME, "detected start up of google services in main profile: killing immeadiatly");
          kill_by_user(UID_GSERVICES);
       }
       if(get_proc_count(UID_GSERVICES2)>0&&UID_GSERVICES2>0){
          __android_log_print(ANDROID_LOG_INFO, MODNAME, "detected start up of gapps in main profile: killing immeadiatly");
          kill_by_user(UID_GSERVICES2);
       }
#endif 
       sleep(IDLE_TIME);

    }
}
