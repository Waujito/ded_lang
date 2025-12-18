#!/bin/bash

set -e

PROGRAM_NAME=$1

if [ ! -e "$PROGRAM_NAME" ]; then
    echo "File $PROGRAM_NAME does not exist"
    exit 1
fi

./frontend/build/frontend "$PROGRAM_NAME"
./middleend/build/middleend "$PROGRAM_NAME.ast" "$PROGRAM_NAME.simpl.ast"
./backend/build/backend "$PROGRAM_NAME.simpl.ast" "$PROGRAM_NAME.asm"
../ded_spu/build/translator "$PROGRAM_NAME.asm" > "$PROGRAM_NAME.o"
