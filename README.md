# ug2extract
A small command-line utility to extract meshes from Need for Speed: Underground 2.
The program currently takes exactly 1 argument, which is a path to one of the `GEOMETRY.bin` files
inside the game's `CARS` directory. Afterwards the program creates an `out` folder and puts all of
the extract geometry files in there in the form of Wavefront OBJ files that you can directly import
into Blender or whatever modeling program of your choice.

## Disclaimer
The program does not contain any assets of the game itself. In order to use the program you must own
a legal copy of Need for Speed: Underground 2.