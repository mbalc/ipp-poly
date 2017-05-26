#!/bin/bash

if [[ $# -ne 2 ]]; then
    echo "zła liczba argumentów"
fi

PROGRAM=$1
DATADIR=${2%/}

if [[ ! -x "$PROGRAM" ]]; then
    echo "podana nazwa programu nie wskazuje na plik wykonywalny"
fi

if [[ ! -d "$DATADIR" ]]; then
    echo "podana nazwa programu nie wskazuje na katalog"
fi

IFS=$'\n'
for file in `find $DATADIR -type f`; do
    if [[ "$(head -n 1 $file)" == "START" ]]; then
        STARTFILE=$file
    fi
done


WORKDIR=`mktemp -d -p $DATADIR`
tail -n +2 "$STARTFILE" > $WORKDIR/calc.in
touch $WORKDIR/calc.out

while : ; do
    EXECUTIVE=`tail -n 1 "$WORKDIR/calc.in"`
    head -n -1 "$WORKDIR/calc.in" >> "$WORKDIR/calc.out"
    cat "$WORKDIR/calc.out" > "$WORKDIR/calc.in"
    ./$PROGRAM < "$WORKDIR/calc.in" > "$WORKDIR/calc.out"
    if [[ $EXECUTIVE == "STOP" ]]; then
        break;
    else
        if [[ `echo $EXECUTIVE | awk '{print $1;}'` == "FILE" ]]; then
            cat "$DATADIR/$(echo $EXECUTIVE | cut -d ' ' -f2-)" > "$WORKDIR/calc.in"
        else
            echo "parse error"
            break
        fi
    fi
done

cat "$WORKDIR/calc.out"

rm -rf "$WORKDIR"
