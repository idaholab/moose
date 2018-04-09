# Function Series

!syntax description /Functions/FunctionSeries

## Description

This `Function` is the workhorse of the Functional Expansion Tools module. It
bridges the gap between the functional series and the associated
coefficients. For example, it provides access to the orthonormalized function
series that the `UserObject` classes need to evaluate a functional expansion. It
also provides the standard function series that are required to couple FXs to
the solvers (`AuxKernels`, `BCs`, etc...). As such, it is configured with
correlation parameters. These includes physical boundaries, series types, and
series' orders.

The coefficients are provided through implementing `MutableCoefficientsFunctionInterface`, and the function series are provided by composition (member variable) via `CompositeSeriesBasisInterface`.

!alert note title=Series normalization
There are `generation_type` and `expansion_type` parameters provided by `FunctionSeries`
that are passed on to the single series (`Zernike`, `Legendre`, ...). The
aforementioned parameters are `MooseEnum`s with (at the time of writing)
possible values of `orthonormal`, `standard`, and `sqrt_mu`. If the user does
not make any specification in the input file, then `generation_type` defaults to
`orthonormal` and `expansion_type` defaults to `standard`. Note that the
combination of generation and expansion must together apply the square of the
orthonormalization coefficient (see the work of David Griesheimer for more
detail).   `sqrt_mu` series generation/expansion
applies the orthonormalization constant. `orthonormal` series
generation/expansion applies the square of the
orthonormalization constant.  `standard` series generation/expansion
does not apply any factor. Consequently, valid combinations include: 1)
`orthonormal` for generation and `standard` for expansion (the default), 2)
`sqrt_mu` for both generation and expansion, and 3) `standard` for generation
and `orthonormal` for expansion. We suggest that users stick with the defaults
unless coupling with external codes that require different normalizations.


## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/1D_volumetric_Cartesian/main.i block=Functions id=1DCart caption=Example use of 1D Cartesian FunctionSeries

!listing modules/functional_expansion_tools/examples/3D_volumetric_Cartesian/main.i block=Functions id=3DCart caption=Example use of 3D Cartesian FunctionSeries

!listing modules/functional_expansion_tools/examples/3D_volumetric_cylindrical/main.i block=Functions id=3DCyl caption=Example use of 3D cylindrical FunctionSeries

!listing modules/functional_expansion_tools/test/tests/standard_use/volume_coupling_custom_norm.i block=Functions id=CustomNorm caption=Example of specifying custom normalization

!syntax parameters /Functions/FunctionSeries

!syntax inputs /Functions/FunctionSeries

!syntax children /Functions/FunctionSeries
