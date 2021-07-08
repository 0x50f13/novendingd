#!/bin/bash

export PATH=$PATH:/bin/:/sbin/:/usr/sbin/:/usr/bin/

cd $(dirname `which $0`)
BASEDIR=`pwd`
echo $BASEDIR
# Colors
if test -t 1; then
    bold=$(tput bold)
    normal=$(tput sgr0)
    red=$(tput setaf 1)
    green=$(tput setaf 2)
    blue=$(tput setaf 4)
    yellow=$(tput setaf 11)
else
    echo "[!]: No colors will be available: not supported."
fi

error(){
    echo "${bold}${red}[-]:${normal}${@}"
}

success(){
    echo "${bold}${green}[+]:${normal}${@}"
}

warn(){
    echo "${bold}${yellow}[!]:${normal}${@}"
}

info(){
    echo "${bold}${blue}[*]:${normal}${@}"
}

exec(){
    info "${@}"
    eval "${@}"
    code=$?
    if [ ! $code -eq 0 ]; then
        error "Exec: ${@} failed with non-zero exit code: ${code}"
        exit -1
    fi
}

require_directory(){ # Creates directory if not exist
    if [ ! -d $1 ]; then
        warn "Directory not found: ${1}. Will create it now."
        exec "mkdir ${1}"
    fi
}

change_dir(){
    if [ ! -d $1 ]; then
        error "No such directory: ${1}"
        exit -1
    fi
    info "Entering directory: ${1}"
    cd $1
}

leave_dir(){
    info "Leaving directory: $(pwd)"
    cd "$BASEDIR"
}

check_equal(){
    printf "${bold}${blue}[*]:${normal}Checking equality: ${1} and ${2}..."
    if ! cmp $1 $2 >/dev/null 2>&1
    then
      printf "${bold}${red}FAILED${normal}\n"
      return -1
    fi
    printf "${bold}${green}OK${normal}\n"
    return 0
}

require_equal(){
    check_equal $1 $2
    local r=$?
    if [ ! $r -eq 0 ]; then
       error "Files ${1} and ${2} are not equal."
       exit -1
    fi
}

check_command(){
    printf "${bold}${blue}[*]:${normal}Checking that ${1} avail..."
    if ! [ -x "$(command -v ${1})" ]; then
      printf "${bold}${red}FAILED${normal}\n"
      return -1
    fi
    printf "${bold}${green}OK${normal}\n"
    return 0
}

require_command(){
    check_command $1
    local r=$?
    if [ ! $r -eq 0 ]; then
       error "Command ${1} is not avail."
       exit -1
    fi
}

require_root(){
    if [ "$EUID" -ne 0 ]; then 
      error "Root is required to run this script"
      exit -1
    fi
}

if [ $# -eq 0 ]; then
    error "Please specify target. Use -h option for help."
    exit -1
fi

source Buildfile

if [[ $1 == "-h" ]]; then
    echo "Usage:"$0" <-h> $USAGE"
    echo "-h        Display this help message and exit."
    exit 0
fi



if [[ `type -t "target_${1}"` == "function" ]]; then
     eval "target_${1}"
     success "Done."
else
     error "No such target:${1}."
fi
