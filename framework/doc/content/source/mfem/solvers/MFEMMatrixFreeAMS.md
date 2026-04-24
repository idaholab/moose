# MFEMMatrixFreeAMS

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::MatrixFreeAMS` low-order refined (LOR) solver to use as a matrix-free
preconditioner for a curl-curl problem of the form

!equation
(\alpha \vec \nabla \times \vec u, \vec \nabla \times \vec v)_\Omega +
(\beta \vec u, \vec v)_\Omega
= (\vec f, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ and $\alpha$ and $\beta$ are scalar coefficients.

The number of CG iterations to use for inner solves on the auxiliary spaces associated with the
Nédélec interpolation operator $\Pi$ and the gradient operator $G$ can be controlled by the
`inner_pi_iterations` and `inner_g_iterations` parameters, which default to 0 and 1, respectively.
Increasing these may aid convergence when $\alpha$ and/or $\beta$ are highly discontinuous.

The method used corresponds to a matrix-free version of Hypre's AMS preconditioner (with default
cycle 1), as described in Hypre's [AMS documentation](https://hypre.readthedocs.io/en/latest/solvers-ams.html).

Implementation details for the `mfem::MatrixFreeAMS` preconditioner, along with some performance
comparisons against Hypre's AMS preconditioner for 2D problems, can be found in
[this paper](https://www.osti.gov/servlets/purl/1835018).

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=Solvers

!syntax parameters /Solvers/MFEMMatrixFreeAMS

!syntax inputs /Solvers/MFEMMatrixFreeAMS

!syntax children /Solvers/MFEMMatrixFreeAMS

!if-end!

!else
!include mfem/mfem_warning.md
