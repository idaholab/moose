# ElementVariablesDifferenceMax

!syntax description /VectorPostprocessors/ElementVariablesDifferenceMax

This postprocessor can find the maximum of the difference or the absolute difference if the
[!param](/VectorPostprocessors/ElementVariablesDifferenceMax/furthest_from_zero) parameter is set to `true`.
This vector postprocessor output to CSV has the following columns, in this order by default:

- the maximum difference between the two variables

- the value of variable A at the location of the maximum difference

- the value of variable B at the location of the maximum difference

- the first (X in Cartesian) coordinate of the location of the maximum difference

- the second (Y in Cartesian) coordinate of the location of the maximum difference

- the third (Z in Cartesian) coordinate of the location of the maximum difference

!alert note title=Vector names / CSV output column names
The names of the vectors declared are `Difference`, the name of the [!param](/VectorPostprocessors/ElementVariablesDifferenceMax/compare_a) variable,
the name of the [!param](/VectorPostprocessors/ElementVariablesDifferenceMax/compare_b) variable, and finally `X`, `Y`, `Z` for the location of the maximum difference.

## Example input syntax

In this example, we compare variable `u` and `v` using a `ElementVariablesDifferenceMax` vector postprocessor.

!listing test/tests/vectorpostprocessors/element_variables_difference_max/element_variables_difference_max.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/ElementVariablesDifferenceMax

!syntax inputs /VectorPostprocessors/ElementVariablesDifferenceMax

!syntax children /VectorPostprocessors/ElementVariablesDifferenceMax
