abr2png
=======

Convert Photoshop Brush (.abr) and PaintShopPro (.jbr) to Portable Grayscale (.pgm)  or Portable Network Graphics (.png) format.

Modified slightly to support different export methods:
* `-png`:  outputs a normal grey-scale PNG of each brush image
* `-cc3`:  outputs a pair of PNGs for each brush image, usable within Campaign Cartographer 3+ as varicolor symbols
* `-wonder`: outputs a PNG for use as a colorable sprite in Wonderdraft
  
# Usage Example
1) Download a brush from the wonderful [KM Alexander site](https://kmalexander.com/free-stuff/fantasy-map-brushes/).
2) On the command line, run `abr2png.exe -cc3 brushfilename.abr`
3) All the PNGs will be extracted into the current directory.

#Credits
Unix fork of [abr2pgm](https://code.google.com/p/abr2pgm/)

