#! /bin/sh

./midi2array $1
touch list.txt
sed "s/^0_0_0_0$/file '0.png'/g; s/^0_0_0_80$/file '1.png'/g ; s/^0_0_80_0$/file '2.png'/g ; s/^0_0_80_80$/file '3.png'/g ; s/^0_80_0_0$/file '4.png'/g ; s/^0_80_0_80$/file '5.png'/g ; s/^0_80_80_0$/file '6.png'/g ; s/^0_80_80_80$/file '7.png'/g ; s/^80_0_0_0$/file '8.png'/g; s/^80_0_0_80$/file '9.png'/g ; s/^80_0_80_0$/file '10.png'/g ; s/^80_0_80_80$/file '11.png'/g ; s/^80_80_0_0$/file '12.png'/g ; s/^80_80_0_80$/file '13.png'/g ; s/^80_80_80_0$/file '14.png'/g ; s/^80_80_80_80$/file '15.png'/g " ./midi_array.txt > list.txt
ffmpeg -r 60 -f concat -i list.txt  out.mp4
rm list.txt
rm midi_array.txt
