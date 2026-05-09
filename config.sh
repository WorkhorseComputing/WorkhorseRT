#!/usr/bin/env bash

set -e

PY="python3"

AUTOCONF_H_DIR="include/generated"
AUTOCONF_H="autoconf.h"

AUTOCONF_DIR="generated"
AUTOCONF_CMAKE="autoconf.cmake"
AUTOCONF_LD="autoconf.ld"

GENHEADER_PY="scripts/config/genConfigHeader.py"
GENCMAKE_PY="scripts/config/genConfigCmake.py"
GENLD_PY="scripts/config/genConfigLd.py"

CMD=$1

case "$CMD" in

    menuconfig)
        $PY -m menuconfig
        ;;
        
    alldefconfig)
        $PY -m alldefconfig
        ;;

    oldconfig)
        $PY -m oldconfig
        ;;

    genconfig)
        mkdir -p $AUTOCONF_H_DIR
        mkdir -p $AUTOCONF_DIR

        $PY $GENHEADER_PY > "$AUTOCONF_H_DIR/$AUTOCONF_H"
        $PY $GENCMAKE_PY > "$AUTOCONF_DIR/$AUTOCONF_CMAKE"
        $PY $GENLD_PY > "$AUTOCONF_DIR/$AUTOCONF_LD"
        ;;

    help | "")
        echo "Usage: ./configure.sh [menuconfig | alldefconfig | oldconfig | genconfig]"
        exit 0
        ;;

    *)
        echo "config.sh: invalid option -- '$CMD'"
        echo "Try 'bash config.sh help' for more information."
        ;;
esac
