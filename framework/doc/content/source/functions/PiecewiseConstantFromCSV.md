# PiecewiseConstantFromCSV

!syntax description /Functions/PiecewiseConstantFromCSV

## Description

The `PiecewiseConstantFromCSV` function is used to load data from a CSV file into a function. The
[PropertyReadFile.md] user object takes care of reading the CSV file, and the function queries
information from it.

It can assume CSV data

- is sorted by element-id, in which case, when the function is evaluated at a point, it will locate the element containing it then return the value for that element in the CSV file
- is sorted by node-id, in which case, when the function is evaluated at a point, it will locate the node at that point then return the value for that node in the CSV file
- is sorted by blocks, in which case, when the function is evaluated at a point, it will locate the element containing it then return the value for that element's block in the CSV file
- defines an interpolation grid, with the voronoi [!param](/Functions/PiecewiseConstantFromCSV/read_type), in which case the function will locate the closest point in that interpolation grid, then return the value for that point in the CSV file


For the latter case, the first three columns of the CSV data must define the coordinates of each point forming the interpolation grid. The [!param](/Functions/PiecewiseConstantFromCSV/column_number) parameter should still match the actual column number in the CSV file, so it likely should be larger than 3 (the number of columns for voronoi centers coordinates).

!alert note
When use data by block or by element, if there is multiple possibilities for the element to choose from, for example at a node,
the element with the lowest ID will be used.

!alert note
The [!param](/Functions/PiecewiseConstantFromCSV/column_number) parameter assumes 0-based indexing of the columns in the CSV file. If you want the values from the leftmost column in the file, you must use a column number of `0`.

## Example Input Syntax

In this example, we display four options for using CSV data to compute a function over an unstructured mesh:

- the `element` Function, using the `reader_element` user object, assumes the CSV file is sorted by element ID, and returns the value of the element containing each point
- the `node` Function, using the `reader_node` user object, assumes the CSV file is sorted by node ID, and returns the corresponding value at those nodes. Outside of these nodes, the function is currently set to error.
- the `nearest` Function, using the `reader_nearest` user object, finds the closest point defined in the CSV file, and returns the corresponding value
- the `block` Function, using the `reader_block` user object, assumes the data in the CSV file is sorted by block, and returns the value corresponding to the block containing each point


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
