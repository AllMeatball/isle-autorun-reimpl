#!/bin/sh

# clear out bitmaps.h
echo "#ifndef _RES_BITMAPS_H" > bitmaps.h
echo "#define _RES_BITMAPS_H" >> bitmaps.h

for f in *.bmp; do
    GEN_FILE_NAME=$(echo "$f" | tr "." "_")
    GEN_FILE_NAME="${GEN_FILE_NAME}.h"

    echo "Generating $GEN_FILE_NAME..."
    xxd -i "$f" > "$GEN_FILE_NAME"

    echo "#include \"$GEN_FILE_NAME\"" >> bitmaps.h
done
echo "#endif" >> bitmaps.h
