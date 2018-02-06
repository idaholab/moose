# Function Series

!syntax description /Functions/FunctionSeries

## Description

This `Function` is the workhorse of the Functional Expansion Tools module. It bridges the gap between the functional series and the associated coefficients. For example, it provides access to the orthonormalized function series that the `UserObject` classes need to evaluate a functional expansion. It also provides the standard function series that are required to couple FXs to the solvers (`AuxKernels`, `BCs`, etc...).

As such, it is configured with correlation parameters. These includes physical boundaries, series types, and series' orders.

The coefficients are provided through implementing `MutableCoefficientsFunctionInterface`, and the function series are provided by composition (member variable) via `CompositeSeriesBasisInterface`.

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/1D_volumetric_Cartesian/main.i block=Functions id=1DCart caption=Example use of 1D Cartesian FunctionSeries

!listing modules/functional_expansion_tools/examples/3D_volumetric_Cartesian/main.i block=Functions id=3DCart caption=Example use of 3D Cartesian FunctionSeries

!listing modules/functional_expansion_tools/examples/3D_volumetric_cylindrical/main.i block=Functions id=3DCyl caption=Example use of 3D cylindrical FunctionSeries

!syntax parameters /Functions/FunctionSeries

!syntax inputs /Functions/FunctionSeries

!syntax children /Functions/FunctionSeries
