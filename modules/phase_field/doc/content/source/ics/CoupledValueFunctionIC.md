# CoupledValueFunctionIC

!syntax description /ICs/CoupledValueFunctionIC

CoupledValueFunctionIC initializes the value of a variable with the value of a
coupled MOOSE Function which is evaluated with a set of up to four coupled
variable values `v` as its input parameters. The coupled variable values are
substituted for the `x`,`y`,`z`, and `t` function variables in that order.

One example application is the use of a
[`PiecewiseMultilinear`](PiecewiseMultilinear.md) function with a data file
containing a pretabulation of dependent variable values as a function of up to
four primary variable values.

This capability can be used when internal degrees of freedom, such as phase
concentrations in a [KKS model](KKS.md), need to be initialized to a good
initial guess to improve the convergence of the first timestep. In the KKS case,
and physical IC will be applied to the primary global alloy concentration
variables, and the phase concentrations will be initialized from pretabulated
data containing good approximations of the phase concentrations for each primary
global alloy concentration.

!syntax parameters /ICs/CoupledValueFunctionIC

!syntax inputs /ICs/CoupledValueFunctionIC

!syntax children /ICs/CoupledValueFunctionIC

!bibtex bibliography
