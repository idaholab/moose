# CSVReaderVectorPostprocessor

The CSVReaderVectorPostprocessor reads [CSV](https://en.wikipedia.org/wiki/Comma-separated_values) data from a file and
converts each column into a VectorPostprocessor vector. This object uses the
[DelimitedFileReader](MooseUtils.md#delimitedfilereader) utility to perform the reading of the file.

The names of the vectors declared by the `CSVReaderVectorPostprocessor` are the names of the columns in the CSV file.

## Example Input Syntax

In this example, the `example.csv` file containing data for year/month/day is being read by
the `CSVReaderVectorPostprocessor`.

!listing test/tests/vectorpostprocessors/csv_reader/read.i block=VectorPostprocessors

!listing test/tests/vectorpostprocessors/csv_reader/example.csv

!syntax parameters /VectorPostprocessors/CSVReaderVectorPostprocessor

!syntax inputs /VectorPostprocessors/CSVReaderVectorPostprocessor

!syntax children /VectorPostprocessors/CSVReaderVectorPostprocessor
