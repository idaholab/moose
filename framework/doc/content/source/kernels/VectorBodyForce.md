# VectorBodyForce

## Description

`VectorBodyForce` is analagous to [`BodyForce`](/BodyForce.md)
except it is applied to vector finite element variables. Instead of taking one
`function` parameter, the user can pass up to three functions representing the
individual components of the vector variable. If not supplied by the user,
`function_x` will default to '1', while `function_y` and `function_z` default to '0'.

!syntax parameters /Kernels/VectorBodyForce

!syntax inputs /Kernels/VectorBodyForce

!syntax children /Kernels/VectorBodyForce
