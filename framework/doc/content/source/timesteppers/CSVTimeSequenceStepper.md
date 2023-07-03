# CSVTimeSequenceStepper

This time stepper derives from [TimeSequenceStepperBase.md] and provides the
sequence of time values from a CSV file or similarly delimited text file.
A [DelimitedFileReader](MooseUtils.md#delimitedfilereader) is used
to read the CSV file.

The file is always read in columns. The column can either be accessed by name
(using the "column_name" parameter, provided that the CSV file has a header
containing the names of the different columns) or by index (using the
"column_index" parameter, with 0 the index of the first column).

See [TimeSequenceStepperBase.md#failed_solves] for information on the behavior
of this time stepper for failed time steps.

!syntax parameters /Executioner/TimeSteppers/CSVTimeSequenceStepper

!syntax inputs /Executioner/TimeSteppers/CSVTimeSequenceStepper

!syntax children /Executioner/TimeSteppers/CSVTimeSequenceStepper
