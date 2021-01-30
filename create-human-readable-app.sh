#!/bin/bash

if [ ! -f "$1" ] ; then

    echo "ERROR: Failed to open file: $1"
    exit 1
fi


padding=0
w=$((4096 - 32*2))
h=$((2160 - 32*2))
dw=$(($w - $padding*2))
dh=$(($h - $padding*2))

# Font height in pixels
pointsize=20
font="FreeMono"
width=`convert -debug annotate  xc: -font $font -pointsize $pointsize -annotate 0 '0' null: 2>&1 | grep Metrics: | sed -e 's/.*width: \([0-9]*\); .*/\1/g'`

rm -rf out
mkdir out
./ivm-hex-dump --font "$width""x""$pointsize" --width $dw --height $dh -o ./out $1


for i in out/*.txt ; do
    convert \
        -font $font -pointsize $pointsize \
        -page "$w""x""$h+$padding+$padding" \
        text:$i \
        -depth 8 -border 0 \
        $(dirname $i)/$(basename $i .txt).png
done

identify out/*.png


