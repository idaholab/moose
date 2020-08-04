# PODReducedBasisSurrogate

!syntax description /Surrogates/PODReducedBasisSurrogate

## Overview

This surrogate takes the reduced operators and bases from [PODReducedBasisTrainer.md]
and assembles the reduced equation system for a new parameter sample ($\boldsymbol{\mu^* }$):

!equation id=rom_affine_decomp_final
\left(\sum \limits_{i=1}^{N_A} f^A_i(\boldsymbol{\mu^* })\textbf{A}_i^r\right)\textbf{c}(\boldsymbol{\mu^* }) =
\sum \limits_{i=1}^{N_b} f^b_i(\boldsymbol{\mu^* })\textbf{b}_i^r.

Following this, the reduced equation system is solved for $\textbf{c}(\boldsymbol{\mu^* })$ and
an approximate solution is reconstructed as

!equation id=full_sol_approximation
\textbf{u}(\boldsymbol{\mu})
\approx
\boldsymbol{\Phi}\textbf{c}(\boldsymbol{\mu}).

It must be mentioned that in case of Dirichlet boundary conditions (either nodal or in weak form)
there is a a contribution in $\textbf{A}_{Dir}^r$ and $\textbf{b}_{Dir}^r$ as well.
However, in this case a penalty parameter ($\gamma$) is used to enforce the boundary condition at
reduced order level. Therefore, the slightly modified equation system can be written as:

!equation id=rom_affine_decomp_final
\left(\sum \limits_{i=1}^{N_A-1} f^A_i(\boldsymbol{\mu^* })\textbf{A}_i^r+\gamma f^A_{Dir}(\boldsymbol{\mu^* })\textbf{A}_{Dir}^r\right)\textbf{c}(\boldsymbol{\mu^* }) =
\sum \limits_{i=1}^{N_b-1} f^b_i(\boldsymbol{\mu^* })\textbf{b}_i^r+\gamma f^b_{Dir}(\boldsymbol{\mu^* })\textbf{b}_{Dir}^r.

the magnitude of $\gamma$ can be set using the [!param](/Surrogates/PODReducedBasisSurrogate/penalty) parameter in the input file. It is important
to note that by increasing the magnitude of $\gamma$, the condition number of the reduced equation
may deteriorate, therefore the overly high values are not recommended.

It is important to mention that the size of the reduced system can be modified by
changing the number of bases per variable in the input file. To do this one can
use [!param](/Surrogates/PODReducedBasisSurrogate/change_rank) and [!param](/Surrogates/PODReducedBasisSurrogate/new_ranks) input parameters. This feature makes it possible
to test the accuracy of the surrogate with different subspace sizes without the
need of rerunning the training procedure.

## Example Input File Syntax

To create a POD reduced basis surrogate model, one can use the following syntax:

!listing pod_rb/internal/surr.i block=Surrogates

It is visible that the reduced operators and basis vectors from [PODReducedBasisTrainer.md]
have been saved to `trainer_out_pod_rb.rd` and the surrogate model is constructed by
loading the necessary information from it. For the sampling
of the uncertain parameters, the same objects can be used in the [Samplers](Samplers/index.md) block:

!listing pod_rb/internal/surr.i block=Samplers

Finally, a vector postprocessor of type `PODSurrogateTester` is created to extract
the approximate value of the nodal maximum of variable `u` in this case:

!listing pod_rb/internal/surr.i block=VectorPostprocessors



!syntax parameters /Surrogates/PODReducedBasisSurrogate

!syntax inputs /Surrogates/PODReducedBasisSurrogate

!syntax children /Surrogates/PODReducedBasisSurrogate
