# FunctionLayeredIntegral

!syntax description /UserObjects/FunctionLayeredIntegral

## How to define the layers

The parameters to define layers are explained in the
[LayeredAverage](/userobjects/LayeredAverage.md) documentation.

## How to retrieve the result

The result of a `FunctionLayeredIntegral` computation can be saved in an auxiliary variable using a
[SpatialUserObjectAux](/auxkernels/SpatialUserObjectAux.md).
It can be output to a CSV file using a [SpatialUserObjectVectorPostprocessor](/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.md).

## Example Input File Syntax

In this example, the integral of a function `sin(y)` is taken over the
whole domain in the `y` direction over 20 layers. The result of the integral is stored
in the variable `layered_integral` using a
[SpatialUserObjectAux](/auxkernels/SpatialUserObjectAux.md), and output to a
CSV file using a [SpatialUserObjectVectorPostprocessor](/vectorpostprocessors/SpatialUserObjectVectorPostprocessor.md).

!listing test/tests/userobjects/function_layered_integral/function_layered_integral.i
  start=AuxKernels
  end=Executioner

!syntax parameters /UserObjects/FunctionLayeredIntegral

!syntax inputs /UserObjects/FunctionLayeredIntegral

!syntax children /UserObjects/FunctionLayeredIntegral
