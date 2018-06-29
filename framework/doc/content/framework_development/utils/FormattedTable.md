# FormattedTable

The FormattedTable object is a general purpose utility for holding table data. Typically it's used for holding [PostProcessor](syntax/Postprocessors/index.md)
values or scalar variables produced by [ScalarKernels](/ScalarKernels/index.md). It provides several useful utilities for displaying data and efficiently
outputting the data.

Example of output:

```
+----------------+----------------+
| time           | num_dofs       |
+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |
|   1.000000e-01 |   5.780000e+02 |
|   2.000000e-01 |   8.500000e+02 |
+----------------+----------------+
```

The number of rows displayed in the table is controlled by the Console parameter `max_rows` (default: 15):

```
[Outputs]
  [./console]
    type = Console
    max_rows = 10
  [../]
[]
```

When the maximum number of rows are displayed, the table shows only the <max_rows> most recent rows of the table and
indicates that rows have been omitted with a "sideways ellipsis" at the top of the table. e.g.:

```
+----------------+----------------+
| time           | num_dofs       |
+----------------+----------------+
:                :                :
|   1.000000e-01 |   5.780000e+02 |
|   2.000000e-01 |   8.500000e+02 |
|   3.000000e-01 |   1.146000e+03 |
|   4.000000e-01 |   8.500000e+02 |
|   5.000000e-01 |   1.146000e+03 |
|   6.000000e-01 |   8.500000e+02 |
|   7.000000e-01 |   1.146000e+03 |
|   8.000000e-01 |   8.500000e+02 |
|   9.000000e-01 |   1.146000e+03 |
|   1.000000e+00 |   8.500000e+02 |
+----------------+----------------+
```

## Controlling Table Width

The FormattedTable object can also respect a maximum width parameter. When running serially, the table can read the width
of the terminal and break the table into multiple pieces for better readability. Unfortunately, the terminal width parameter
is not available when running through MPI or any other batch mode, which is typical when running on a cluster. The table
width can also be controlled by setting an environment variable `MOOSE_PPS_WIDTH` to the desired width (in characters).

## File Output

The FormattedTable writes to CSV files natively and efficiently. Only one process will attempt to open and output to the specified
CSV file. The table object also caches its filehandle so that several writes to the same file do not incur the costs to
open and close the file. Only data that has not already been output is output on each command to write data to a file, reducing
the I/O overheads. Finally, the object implements the templated dataStore/dataLoad routines so that it is restart aware
when using MOOSE's restart system.

### Normal use within MOOSE

Several instances of FormattedTable exist inside of the [TableOutput](/TableOutput.md) object. Each of these are declared
as "restartable data" meaning that they will append to existing files when MOOSE is run in `--recover` mode.

There are two main interfaces that one can use when interacting with the FormattedTable object. The simple interface is to call
the overloaded "addData()" method that takes the independent variable ("time") to add individual values to the table at specific locations:

!listing framework/include/utils/FormattedTable.h re=void\saddData.*?time\); re-flags=re.U

The more advanced interface is to insert new rows followed by specific row values without the independent variable:

!listing framework/include/utils/FormattedTable.h line=addRow

!listing framework/include/utils/FormattedTable.h re=void\saddData.*?value\); re-flags=re.U
