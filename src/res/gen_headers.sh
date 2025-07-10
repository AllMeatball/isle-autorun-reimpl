#!/bin/sh

EXPORT_PATH="media.h"

# clear out media.h
echo "#ifndef _RES_BITMAPS_H" > "$EXPORT_PATH"
echo "#define _RES_BITMAPS_H" >> "$EXPORT_PATH"

for f in *.bmp *.smk; do
    GEN_FILE_NAME=$(echo "$f" | tr "." "_")
    GEN_FILE_NAME="${GEN_FILE_NAME}.h"

    echo "Generating $GEN_FILE_NAME..."
    xxd -i "$f" > "$GEN_FILE_NAME"

    echo "#include \"$GEN_FILE_NAME\"" >> "$EXPORT_PATH"
done
echo "#endif" >> "$EXPORT_PATH"
