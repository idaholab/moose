# CauchyStressFromNEML2UO

!syntax description /UserObjects/CauchyStressFromNEML2UO

## Description

This object uses the specified NEML2 material model to perform mesh-wise batched
material update. The input material properties and variables at all quadrature
points on the mesh are first gathered together, then the NEML2 material model is
used to map the inputs (strain, temperature, time, etc.) to the outputs (stress
etc.). The material model's forward operator is defined in the NEML2 input file.
The outputs are gathered into an internal data structure, and a
[`CauchyStressFromNEML2Receiver`](CauchyStressFromNEML2Receiver.md optional=True)
object is required to assign the outputs to their corresponding material properties.

## Example Input Syntax

!syntax parameters /UserObjects/CauchyStressFromNEML2UO
