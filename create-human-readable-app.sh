#!/bin/bash

w=$1
h=$2
pointsize=$3
out=$4
in=$5

if [ "$#" != "5" ] ; then
    echo "Usage: $0 <image-width> <image-height> <fontsize> <out-dir> <input-app> "
    exit 1
fi

if [ ! -f "$in" ] ; then
    echo "ERROR: Failed to open file: $in"
    exit 1
fi

which ivm-hex-dump > /dev/null
if [ "$?" != "0" ] ; then
    echo "ERROR: ivm-hex-dump tool not found"
    exit 1
fi

# Font height in pixels
font="FreeMono"
width=`convert -debug annotate  xc: -density 72 -font $font -pointsize $pointsize -annotate 0 '0' null: 2>&1 | grep Metrics: | sed -e 's/.*width: \([0-9]*\); .*/\1/g'`

rm -rf $out
mkdir $out
# subtract pointsize since convert add bottom border
ivm-hex-dump --font "$width""x""$pointsize" --width $w \
             --height $(($h-$pointsize)) -o $out $in

for i in $out/*.txt ; do
    outimg="$(dirname $i)/$(basename $i .txt).png"
    echo "Creating $outimg"
    convert \
        -font $font -interline-spacing 0 -pointsize $pointsize \
        -page "$w""x""$h+0+0" \
        -gravity center \
        text:$i \
        -depth 8 -bordercolor white +repage \
        $outimg
done

identify $out/*.png
