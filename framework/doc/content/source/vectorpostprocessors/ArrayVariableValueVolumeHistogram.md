# ArrayVariableValueVolumeHistogram

!syntax description /VectorPostprocessors/ArrayVariableValueVolumeHistogram

This VectorPostprocessor lets you tabulate the volumes in the simulation domain where a given array variable has certain values.
The volumes are tabulated for each component.

The CSV output contains the number of components of the array variable plus one column for:

- the variable value bins upper values, with name "value" the first row (header)

- the volumes in which the variable components holds the values within the bins, with the array component names in the first row

Array component names could be specified with [!param](/Variables/ArrayMooseVariable/array_var_component_names) other than their default names.

The names of the vectors declared by `ArrayVariableValueVolumeHistogram` match the CSV output column header names.
It is noted that the number of volumes of each array component is equal to [!param](/VectorPostprocessors/ArrayVariableValueVolumeHistogram/bin_number).
Volumes where variable values are smaller than [!param](/VectorPostprocessors/ArrayVariableValueVolumeHistogram/min_value) or greater than or equal to [!param](/VectorPostprocessors/ArrayVariableValueVolumeHistogram/max_value) are not considered.

## Example input syntax

!syntax parameters /VectorPostprocessors/ArrayVariableValueVolumeHistogram

!syntax inputs /VectorPostprocessors/ArrayVariableValueVolumeHistogram

!syntax children /VectorPostprocessors/ArrayVariableValueVolumeHistogram
