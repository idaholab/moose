#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

## Script which contains the diagnostics we wish to generate.
## This script will output all diagnostics to stdout.

write_to_stdout() {
  echo -e "$@"
}

file_born_date() {
  if [ -f $1 ]; then
    if [ `uname` == "Darwin" ]; then
      echo `stat -f "%Sm" -t "%Y-%m-%d %H:%M" "$1"`
    else
      echo `date -d @$(stat -c %Y "$1") +%Y-%m-%d\ %H:%M`
    fi
  fi
}

diagnose_compilers() {
    # Variable
    which $CC &> /dev/null
    if [ "$?" -eq 0 ]; then
        write_to_stdout "\nVariable \`which \$CC\` check:\n`which $CC`\n\n\$CC --version:\n`$CC --version`"
    else
        write_to_stdout "\n\$CC not set"
    fi

    # Static
    which mpicc &> /dev/null
    if [ "$?" -eq 0 ]; then
        write_to_stdout "\nMPICC:\nwhich mpicc:\n\t`which mpicc`\nmpicc -show:\n\t`mpicc -show`\n\nCOMPILER `echo $(mpicc -show | cut -d\  -f1)`:\n`$(mpicc -show | cut -d\  -f1) --version`"
    else
        write_to_stdout "\nMPICC WRAPPER NOT FOUND"
    fi
}

diagnose_modules() {
    type module &> /dev/null
    if [ "$?" -eq 0 ]; then
        write_to_stdout "\nMODULES:\n`module list 2>&1`"
    else
        write_to_stdout "\nMODULES NOT AVAILABLE"
    fi
}

diagnose_root() {
    if [ $(id -u) = 0 ]; then write_to_stdout "RUNNING AS ROOT"; fi
}

diagnose_environment() {
    write_to_stdout "\nENVIRONMENT:\n`env`"
}

diagnose_cpus() {
    if [ `uname` == "Darwin" ]; then
        CPUS=`/usr/sbin/sysctl -n hw.ncpu`
    else
        CPUS=`cat /proc/cpuinfo | grep processor | wc -l`
    fi
    write_to_stdout "\nCPU Count: `echo $CPUS`"
}

diagnose_memory() {
    if [ `uname` == "Darwin" ]; then
        PAGE_SIZE=`vm_stat | grep "page size of" | sed 's/[^0-9]*//g'`
        MEMORY_FREE=`vm_stat | grep free | awk -v page_size=$PAGE_SIZE '{ print (page_size * $3) / 1048576}'`
    else
        MEMORY_FREE=`awk '/MemFree/ { printf "%.3f \n", $2/1024 }' /proc/meminfo`
    fi
    write_to_stdout "\nMemory Free: `echo $MEMORY_FREE` MB"
}

diagnose_arch() {
    if [ `uname` == "Darwin" ]; then
        SYSTEM=`uname -v`
    else
        which lsb_release &> /dev/null
        if [ "$?" -eq 0 ]; then
            SYSTEM="`lsb_release -a`"
        else
            if [ -f /etc/system-release ]; then
                SYSTEM="`cat /etc/system-release`"
            else
                SYSTEM="Not Available"
            fi
        fi
    fi
    write_to_stdout "\nSystem Arch: `echo $SYSTEM`"
}

diagnose_package_version() {
    if [ -f /opt/moose/build ]; then
        write_to_stdout "\nMOOSE Package Version: `cat /opt/moose/build | sed 's/[^0-9]*//g'`"
    else
        write_to_stdout "\nMOOSE Package Version: Custom Build"
    fi
}

diagnose_python() {
    which python &> /dev/null
    if [ "$?" -eq 0 ]; then
        write_to_stdout "\nPython:\n\t`which python`\n\t`python --version 2>&1`"
    else
        write_to_stdout "\nPython: NO PYTHON DETECTED"
    fi
}

diagnose_petsc() {
    if [ -f "$PETSC_DIR/include/petscconfiginfo.h" ]; then
        formatted=`cat $PETSC_DIR/include/petscconfiginfo.h | sed -e 's|\\\||g'`
        write_to_stdout "\nPETSc configure:\n$formatted"
        write_to_stdout "\nPETSc linked libraries:"

        if [ `uname` = "Linux" ]; then
            ldd $PETSC_DIR/lib/libpetsc.so
        else
            otool -L $PETSC_DIR/lib/libpetsc.dylib
        fi

    elif [ -z "$PETSC_DIR" ]; then
        write_to_stdout "\nPETSC_DIR not set"
    fi
}

# Execute the diagnostics
date
diagnose_root
diagnose_arch
diagnose_package_version
diagnose_cpus
diagnose_memory
diagnose_compilers
diagnose_python
diagnose_modules
diagnose_petsc
diagnose_environment
