# NormalizationAux

!syntax description /AuxKernels/NormalizationAux

The formula for the normalization is

!equation
\dfrac{\text{variable * normal factor}}{\text{normalization}} - \text{shift}

## Example syntax

In this example, the `NormalizationAux` is used to normalize the output of an eigenproblem.
This is a common use case in reactor physics where the neutron flux output by
the numerical solve may not be normalized, and has to be normalized to obtain the desired
power level. The power, used for normalization, is stored in `unorm`, an
`ElementIntegralVariablePostprocessor`.

!listing tests/problems/eigen_problem/eigensolvers/ne_coupled.i block=AuxKernels Postprocessors

!syntax parameters /AuxKernels/NormalizationAux

!syntax inputs /AuxKernels/NormalizationAux

!syntax children /AuxKernels/NormalizationAux
