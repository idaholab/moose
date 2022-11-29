# VariableValueVolumeHistogram

!syntax description /VectorPostprocessors/VariableValueVolumeHistogram

This VectorPostprocessor lets you tabulate the volumes in the simulation domain where a given variable has certain values.

The CSV output contains the two columns for:

- the variable value bins upper values, with the name of the variable in the first row (header)

- the volume in which the variable holds the values within the bins, with the name `n` in the first row

The names of the vectors declared by `VariableValueVolumeHistogram` match the CSV output column header names.

## Example input syntax

!syntax parameters /VectorPostprocessors/VariableValueVolumeHistogram

!syntax inputs /VectorPostprocessors/VariableValueVolumeHistogram

!syntax children /VectorPostprocessors/VariableValueVolumeHistogram
