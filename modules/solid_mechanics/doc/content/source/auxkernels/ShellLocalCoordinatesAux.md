# ShellLocalCoordinatesAux

!syntax description /AuxKernels/ShellLocalCoordinatesAux

!alert note
The three Cartesian local vectors for each shell element are indexed as follows: the first vector is indexed by 0, the second vector by 1, and the normal vector by 2. The convention used to define the direction of these vectors is explained in [ShellElements](/ShellElements.md)



## Example Input Syntax

!listing modules/solid_mechanics/test/tests/shell/static/pinched_cylinder_symm_local_stress.i block=AuxKernels/first_axis_x

!syntax parameters /AuxKernels/ShellLocalCoordinatesAux

!syntax inputs /AuxKernels/ShellLocalCoordinatesAux

!syntax children /AuxKernels/ShellLocalCoordinatesAux
