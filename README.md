# COFAB Slicer

STL slicer we made for the Computional Fabrication course at UHasselt.

## Dependencies

This project requires `Qt>=5.10`, `assimp` and `clipper`.

## Building

This project uses the qmake or cmake build system. To built with cmake:

```
cmake CMakeLists.txt
make -j4
```

With the resulting binary in `bin/`.
