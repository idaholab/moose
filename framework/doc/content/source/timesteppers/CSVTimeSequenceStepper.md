# CSVTimeSequenceStepper

Imports a time sequence from a CSV file, or similar delimited text files.
The CSVTimeSequenceStepper uses a [DelimitedFileReader](MooseUtils.md#delimitedfilereader)
to read the CSV file.

The file is always read in columns. The column can either be accessed by name
(using the "column_name" parameter, provided that the CSV file has a header
containing the names of the different columns) or by index (using the
"column_index" parameter, with 0 the index of the first column).

If the solve fails to converge during a time step, the behavior of the
`CSVTimeSequenceStepper` is the same as the [TimeSequenceStepper.md]. The
time step will be cut then the time stepper will attempt to return to the original sequence.

!syntax parameters /Executioner/TimeStepper/CSVTimeSequenceStepper

!syntax inputs /Executioner/TimeStepper/CSVTimeSequenceStepper

!syntax children /Executioner/TimeStepper/CSVTimeSequenceStepper
