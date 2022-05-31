# PiecewiseConstantFromCSV

!syntax description /Functions/PiecewiseConstantFromCSV

## Description

The `PiecewiseConstantFromCSV` function is used to load data from a CSV file into a function. The
[PropertyReadFile.md] user object takes care of reading the CSV file, and the function queries
information from it.

It can assume CSV data

- is sorted by element-id, in which case, when the function is evaluated at a point, it will locate the element containing it then return the value for that element in the CSV file
- is sorted by blocks, in which case, when the function is evaluated at a point, it will locate the element containing it then return the value for that element's block in the CSV file
- defines an interpolation grid, in which case the function will locate the closest point in that interpolation grid, then return the value for that point in the CSV file


For the latter case, the first columns of the CSV data must define the coordinates of each point forming the interpolation grid. The number of columns used to define these coordinates
must match the dimension of the mesh.

!alert note
When use data by block or by element, if there is multiple possibilities for the element to choose from, for example at a node,
the element with the lowest ID will be used.

## Example Input Syntax

In this example, we display three options for using CSV data to compute a function over an unstructured mesh:

- the `element` function, using the `reader_element` user object, assumes the CSV file is sorted by element ID, and returns the value of the element containing each point
- the `nearest` function, using the `reader_nearest` user object, finds the closest point defined in the CSV file, and returns the corresponding value
- the `block` function, using the `reader_block` user object, assumes the data in the CSV file is sorted by block, and returns the value corresponding to the block containing each point


!listing test/tests/functions/piecewise_constant_from_csv/piecewise_constant.i block=Functions UserObjects

## Other functions which may read data from CSV

These functions read spatial data from CSV, e.g. data that is sorted by location not node, element or block.

- [PiecewiseConstant.md] (1D or time dependence)
- [PiecewiseLinear.md] and [CoarsenedPiecewiseLinear.md] (1D or time dependence)
- [PiecewiseBilinear.md] (2D)
- [PiecewiseMulticonstant.md] (1D to 4D)
- [PiecewiseMultilinear.md] (1D to 4D)
- [Axisymmetric2D3DSolutionFunction.md]

!syntax parameters /Functions/PiecewiseConstantFromCSV

!syntax inputs /Functions/PiecewiseConstantFromCSV

!syntax children /Functions/PiecewiseConstantFromCSV
