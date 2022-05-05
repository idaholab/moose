# DivergenceAux

!syntax description /AuxKernels/DivergenceAux

!alert note
Using this `AuxKernel` to compute a term in a nonlinear equation will discard derivatives when using automatic differentiation (AD), and will make it more difficult to write down the Jacobian contributions when not using AD.

## Example input syntax

In this example, the divergence of a finite volume vector field `(u, v)` is computed over a block, and compared to the flux on the sides of the block, verifying the divergence theorem as a sanity check.

!listing test/tests/auxkernels/divergence_aux/test_fv.i block=AuxKernels Postprocessors

!syntax parameters /AuxKernels/DivergenceAux

!syntax inputs /AuxKernels/DivergenceAux

!syntax children /AuxKernels/DivergenceAux
