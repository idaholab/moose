# PINSFVPorosityVariable

This variable type is specific to the porous media incompressible Navier Stokes
equations. When used instead of a typical finite volume variable, faces for
which the neighboring elements have different porosity values will be treated as
extrapolated boundary faces. E.g. when this variable is queried for a face value
on the "element" side of the face, the element cell center value will be
returned; when this variable is quered for a face value on the "neighbor" side
of the face, the neighbor cell center value will be returned. This design is
meant to ensure that interpolation is not performed between discontinuous values.

!syntax parameters /Variables/PINSFVPorosityVariable

!syntax inputs /Variables/PINSFVPorosityVariable

!syntax children /Variables/PINSFVPorosityVariable
