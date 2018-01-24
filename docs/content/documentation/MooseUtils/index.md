# MooseUtils Namespace
[MOOSE] includes a number of C++ utility classes and functions that may be useful for developing
applications. These utilities are within the MooseUtils C++ namespace. The following summarized
some of the available items.


## DelimitedFileReader
It is often necessary to read data from files during a simulation and often this data is in
a delimited format such as [CSV](https://en.wikipedia.org/wiki/Comma-separated_values). The
DelimitedFileReader is designed for reading this data into application objects.

The DelimitedFileReader can read data organized into columns or rows. It assumes that all data,
outside of the header, is numeric and can be converted to a C++ double. Additionally, it is also
assumed that the first row or column defines the number of columns for the entire file, if the
number of columns differs from the first row an error will be produced.

Within [MOOSE] this utility is utilized by the [CSVReader](framework/CSVReader.md), which is part of
the [VectorPostprocessors] system. This object will be used to explain the use of the utility.

Using the DelimitedFileReader is very simple and requires three steps. First, include the
utility header file with `#include "DelimitedFileReader.h"`. Then the object
is instantiated and read method is called, as shown in the unit test snippet below.

!listing unit/src/DelimitedFileReaderTest.C start=reader("data/csv/example.csv") end=getData strip-leading-whitespace=True

This class is required to include the filename upon construction. Optionally, a second argument
providing a pointer to a [libMesh] Communicator object may be provided. This argument should be
used when the reader is used within a MooseObject. For example, as shown in \ref{csv_reader_ctor},
the CSVReader object passes a Communicator object to the reader. If not provided the reader will
read the data on all processors. If provided it will only read on single processor and broadcast
the data to the others.

It is also possible to configure how file is formatted via the various set methods, as listed below.
The set methods must be called prior to the read method.

* **setIgnoreEmptyLines**<br>
By default this all empty lines are ignored, when set to `false` the presence of an empty line will
cause an error.
* **setFormat**<br>
The reader is capable of reading data organized into columns or rows, this method allows for the
format to be changed.
* **setDelimiter**<br>
By default the reader will attempt to infer the delimiter from the file; however, the detection is
fairly rudimentary. It inspects the file for a comma, if a comma exists then a comma is the
delimiter. If it does not exist then space is the delimiter. The setDelimiter method can be used to
explicitly set string to use for a delimiter.
* **setHeader**<br>
By default the reader will attempt to detect the presence of header strings for both the row and
column data formats. The setHeader method can be used to explicitly control whether header data
exists. If headers exist the each data row or column will be labeled using the header and it will be
accessible via the getData method by name. If no headers exist the header names are generated as
"column_0", "column_1", etc. or "row_0", "row_1", etc. depending on the format settings.
* **setComment**<br>
The string provided to this method marks content that will be ignored. Any line beginning with the
character(s) will be ignored and all characters on a line that follow the character(s) will also
be ignored.

!listing framework/src/vectorpostprocessors/CSVReader.C start=CSVReader:: end=&_communicator include-end=True id=csv_reader_ctor caption=Construction of DelimitedFileReader object within a MooseObject initialization list.

After the data is read using the "read" method, there are two methods used for accessing the data:

* `getNames`: This method returns a vector of the column names as read from the header or
generated based on the number of columns when a header-less file is being examined.
* `getData`: There are three overloaded versions of this method. One returns a reference to the
* entire data set as a vector of vectors. The others access a single vector by name or index.
