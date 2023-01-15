# PiecewiseConstantVariable

When this type of variable is used instead of a typical finite volume variable, faces for
which the neighboring elements have different cell-center values will be treated as
extrapolated boundary faces. E.g. when this variable is queried for a face value
on the "element" side of the face, the element cell center value will be
returned; when this variable is queried for a face value on the "neighbor" side
of the face, the neighbor cell center value will be returned. This variable type
can be used to ensure that interpolation is not performed between potentially
sharply discontinuous values.

!syntax parameters /Variables/PiecewiseConstantVariable

!syntax inputs /Variables/PiecewiseConstantVariable

!syntax children /Variables/PiecewiseConstantVariable
