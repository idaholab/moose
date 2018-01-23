# FunctionSeries
!syntax description /Functions/FunctionSeries

This `Function` is the workhorse of the Functional Expansion Tools module. It bridges the gap between the functional series and the associated coefficients. For example, it provides access to the orthonormalized function series that the `UserObject` classes need to evaluate a functional expansion. It also provides the standard function series that are required to couple FEs to the solvers (`AuxKernels`, `BCs`, etc...).

As such, it is configured with correlation parameters. These includes physical boundaries, series types, and series' orders.

The coefficients are provided through implementing `MutableCoefficientsFunctionInterface`, and the function series are provided by composition (member variable) via `CompositeSeriesBasisInterface`.


!syntax parameters /Functions/FunctionSeries

!syntax inputs /Functions/FunctionSeries

!syntax children /Functions/FunctionSeries
