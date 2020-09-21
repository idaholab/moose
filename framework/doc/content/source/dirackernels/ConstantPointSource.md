# ConstantPointSource

!syntax description /dirackernels/ConstantPointSource

A `ConstantPointSource` reads in a constant point source by its coordinates and value.  Multiple `ConstantPointSource` coordinate and value pairs can be read from a single block or given in an input file.  All three of the following examples are equivalent ways of defining multiple `ConstantPointSource`.  File input must contain all three coordinates, regardless of the dimension of the problem.  

## Example input for single ConstantPointSource

Reading in two seperate single `ConstantPointSource` blocks for a two dimensional problem.  Only the x,y values are given for the coordinates values but three coordiantes with x, y, z=0 could also be given.

!listing test/tests/dirackernels/constant_point_source/2d_point_source.i block=DiracKernels

## Example input for multiple ConstantPointSource read from input

The two seperate single `ConstantPointSource` x, y value pairs are given in a single block using vector input.  The list for the coordinates are given as x$_0$, y$_0$, x$_1$, y$_1$ for corresponding values, v$_0$, v$_1$.

!listing test/tests/dirackernels/constant_point_source/2d_point_source_list.i block=DiracKernels

## Example input for multiple ConstantPointSource read from file

Two seperate single `ConstantPointSource` x,y,z value pairs are read from a csv file given in a single block.

!listing test/tests/dirackernels/constant_point_source/2d_point_source_file.i block=DiracKernels

and the format of the file defining the two point sources looks like:

```
x y z value
0.2 0.3 0 1.0
0.8 0.3 0 2.0
```

The file input must contain all three coordinate values, even for a 2D problem where the z coordinate is set to 0.  Alternatively, to be consistant with the format output by the `NodalValueSampler` `VectorPostprocessors`, nodal ids can be included in the fourth column:


```
x y z id value
0.2 0.3 0 0 1.0
0.8 0.3 0 1 2.0
```

Note that the 4th column containing the nodal ids is completely ignored. 

!syntax parameters /DiracKernels/ConstantPointSource

!syntax inputs /DiracKernels/ConstantPointSource

!syntax children /DiracKernels/ConstantPointSource
