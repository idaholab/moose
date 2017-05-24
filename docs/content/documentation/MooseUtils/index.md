# MooseUtils Namespace
[MOOSE] includes a number of C++ utility classes and functions that may be useful for developing
applications. These utilities are within the MooseUtils C++ namespace. The following summarized
some of the available items.


## DelimitedFileReader
It is often necessary to read data from files during a simulation and often this data is in
a delimited format such as [CSV](https://en.wikipedia.org/wiki/Comma-separated_values). The
DelimitedFileReader is designed for reading this data into application objects.

The DelimitedFileReader assumes assumes that all data, outside of the header row, is numeric and
can be converted to a C++ double. Additionally, it is also assumed that the first row defines the
number of columns for the entire file, if the number of columns differs from the first row an
error will be produced.

Within [MOOSE] this utility is utilized by the [CSVReader](framework/CSVReader.md), which is part of the [VectorPostprocessors] system. This object will be used to explain the use of the utility.

Using the DelimitedFileReader is very simple and requires three steps. First, include the
utility header file with `#include "DelimitedFileReader.h"`. Then the object
is instantiated and read method is called, as shown in the unit test snippet below.

!listing unit/src/DelimitedFileReaderTest.C start=reader("data/csv/example.csv") end=reader.getColumnNames strip-leading-whitespace=True


This class is required to include the filename upon construction and optionally it accepts
additional arguments at construction time, each of the four possible arguments are explained below.

* **Filename** (required)<br>
The first argument expects a valid filename, which does not need to exist until the read method
is called.
* **Header** (optional)<br>
The second argument is a flag (`bool`)---with a default of `true`---that enables or disables the
reading of the header line, by default it is assumed that the file contains a header. If this is
disabled the column names are generated as "column_0", "column_1", etc., thus allowing the
`getColumnNames` to work correctly regardless of the setting for this flag.
* **Delimiter** (optional)<br>
The third argument (`std::string`) is the file delimiter, by default it is assumed to be a comma
(`","`) as is expected for CSV data.
* **Communicator** (optional)<br>
The final argument is a pointer to a [libMesh] Communicator object. This argument should be
used when the reader is used within a MooseObject. For example, as shown in \ref{csv_reader_ctor},
the CSVReader object passes a Communicator object to the reader. If not provided the reader will
read the data on all processors. If provided it will only read on single processor and broadcast
the data to the others.

!listing framework/src/vectorpostprocessors/CSVReader.C start=_csv_reader end=&_communicator include-end=True id=csv_reader_ctor caption=Construction of DelimitedFileReader object within a MooseObject initialization list.

After the data is read using the "read" method, there are two methods used for accessing the data:

* `getColumnNames`: This method returns a vector of the column names as read from the header or
generated based on the number of columns when a header-less file is being examined.
* `getColumnData`: This method returns a vector that contains another vector for each column of
data read from the file, the column vectors correspond to the names returned from `getColumnNames`.
