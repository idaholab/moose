# CSVTimeSequenceStepper

Imports a time sequence from a CSV file, or similar delimited text files. 
The CSVTimeSequenceStepper uses a DelimitedFileReader to read the CSV file 
(see MooseUtils/index.md for more details).

The file is always read in columns. The column can either be accessed by name 
(using the "column_name" parameter, provided that the CSV file has a header 
containing the names of the different columns) or by index (using the 
"column_index" parameter, with 0 the index of the first column).

!syntax description /Executioner/TimeStepper/CSVTimeSequenceStepper

!syntax parameters /Executioner/TimeStepper/CSVTimeSequenceStepper

!syntax inputs /Executioner/TimeStepper/CSVTimeSequenceStepper

!syntax children /Executioner/TimeStepper/CSVTimeSequenceStepper
