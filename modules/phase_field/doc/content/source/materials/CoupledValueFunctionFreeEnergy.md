# CoupledValueFunctionFreeEnergy

!syntax description /Materials/CoupledValueFunctionFreeEnergy

CoupledValueFunctionFreeEnergy evaluates MOOSE Functions with a set of up to
four coupled variable values `v` as its input parameters. The coupled variable
values are substituted for the `x`,`y`,`z`, and `t` function variables in that
order.

Functions for the free energy (optional) and the chemical potentials for the
coupled variables can be supplied. Derivatives of the chemical potentials are
computed automatically using the `gradient` and `timeDerivative` methods of the
`MooseFunction` class.

One example application is the use of
[`PiecewiseMultilinear`](PiecewiseMultilinear.md) functions with data files
containing pretabulations of Gibbs energy and chemical potentials as a function
of up to four primary variable values.

!syntax parameters /Materials/CoupledValueFunctionFreeEnergy

!syntax inputs /Materials/CoupledValueFunctionFreeEnergy

!syntax children /Materials/CoupledValueFunctionFreeEnergy
