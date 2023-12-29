# CauchyStressFromNEML2

!syntax description /Materials/CauchyStressFromNEML2

## Description

This object uses the specified NEML2 material model to perform element-wise batched
material update. On each element, the input material properties and variables at all
quadrature points are first gathered together, then the NEML2 material model is used
to map the inputs (strain, temperature, time, etc.) to the outputs (stress etc.).
The material model's forward operator is defined in the NEML2 input file. Finally,
the outputs are assigned to the corresponding material properties at each quadrature
point.

## Example Input Syntax

!syntax parameters /Materials/CauchyStressFromNEML2
