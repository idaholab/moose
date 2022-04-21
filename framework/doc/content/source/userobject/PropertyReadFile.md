# PropertyReadFile

!syntax description /UserObjects/PropertyReadFile

This user object may load data sorted in four different ways. The read modes are toggled using the
[!param](/UserObjects/PropertyReadFile/read_type) parameter.

- In +element+ mode, the file read contains [!param](/UserObjects/PropertyReadFile/nprop) values for each element in the mesh.
- In +grain+/+voronoi+ mode, a Voronoi tessellation with [!param](/UserObjects/PropertyReadFile/nvoronoi) random centers is either created randomly or read from the first columns of the `CSV` file, depending on the [!param](/UserObjects/PropertyReadFile/use_random_voronoi) parameter. The file read should still contain [!param](/UserObjects/PropertyReadFile/nprop) columns, even though 1-3 columns may be used for the Voronoi tesselation positions.
- In +block+ mode, the file read contains [!param](/UserObjects/PropertyReadFile/nprop) values for each block in the mesh. [!param](/UserObjects/PropertyReadFile/nblock) is the number of blocks in the mesh.
- In +node+ mode, the file read contains [!param](/UserObjects/PropertyReadFile/nprop) values for each node in the mesh.

!alert warning title=Data ordering
For the `element` (`block` and `node` respectively) modes, both the elements
(blocks and nodes respectively) in the mesh and the data in the `CSV` file must be ordered consecutively.
The element (block and node respectively) IDs must be contiguous and usually start from 1.

The [!param](/UserObjects/ElementPropertyReadFile/use_zero_based_block_indexing)
parameter indicates whether the block numbers start with zero (`true`)
or one (`false`).

## Object use

Values can be queried from the object by passing in a property ID and an element
pointer. In +element+ mode a direct lookup from the data table based on
element ID is performed. In +grain+ mode the centroid of the passed in element
is taken and the grain ID is determined as the ID of the Voronoi center closest
to the element centroid.

An example of a MOOSE object using the `PropertyReadFile` is the [PiecewiseConstantFromCSV.md] function.

## Example input syntax

In this example input file, the `PropertyReadFile` user object is used to load data from a CSV file
then a [PiecewiseConstantFromCSV.md] function and a [FunctionIC.md] are used to populate a field with this data.
This is done for each [!param](/UserObjects/PropertyReadFile/read_type).

- Data sorted by element

!listing test/tests/functions/piecewise_constant_from_csv/piecewise_constant.i block=UserObjects/reader_element

!listing test/tests/functions/piecewise_constant_from_csv/data_element.csv

- Data sorted by nearest-neighbor

!listing test/tests/functions/piecewise_constant_from_csv/piecewise_constant.i block=UserObjects/reader_nearest

!listing test/tests/functions/piecewise_constant_from_csv/data_nearest.csv

- Data sorted by block

!listing test/tests/functions/piecewise_constant_from_csv/piecewise_constant.i block=UserObjects/reader_block

!alert note
In this example, the same CSV file as the nearest neighbor case is used. It is however processed and used differently.

- Data sorted by node

!listing test/tests/functions/piecewise_constant_from_csv/piecewise_constant.i block=UserObjects/reader_node

!listing test/tests/functions/piecewise_constant_from_csv/data_node.csv

!syntax parameters /UserObjects/PropertyReadFile

!syntax inputs /UserObjects/PropertyReadFile

!syntax children /UserObjects/PropertyReadFile

!bibtex bibliography
