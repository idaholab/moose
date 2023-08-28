# VectorFromComponentVariablesMaterial

## Description

`VectorFromComponentVariablesMaterial` computes a vector material property with
name specified by the `vector_prop_name` parameter from coupled variable
components. The x-component is computed through the `u` coupled
variable. Optional coupled variables `v` and `w` compute the y- and z-components
respectively.

An example of this object's use is shown in the listing below where in this case
a velocity material property is being declared. The ability to pass constants to
the coupled variables is leveraged in this example. Actual coupled variable
instances would be used in, for example, a Navier-Stokes simulation in which the
nonlinear system solves for velocity components and the vector velocity needs to
be constructed.

!listing test/tests/dgkernels/passive-scalar-channel-flow/test.i block=Materials

!syntax parameters /Materials/VectorFromComponentVariablesMaterial

!syntax inputs /Materials/VectorFromComponentVariablesMaterial

!syntax children /Materials/VectorFromComponentVariablesMaterial
