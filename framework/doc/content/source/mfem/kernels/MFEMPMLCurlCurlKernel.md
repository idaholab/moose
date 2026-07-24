# MFEMPMLCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds a perfectly-matched-layer (PML) stretched curl-curl domain integrator for the bilinear form

!equation
(\mathbf{c}_1 \vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ and

!equation
\mathbf{c}_1 = a\, \det(J)^{-1} J^T J

is the base scalar `coefficient` $a$ scaled by the complex diagonal tensor arising from the PML
coordinate stretch, with $J$ the Jacobian of the stretch (see below). In two dimensions
$\det(J)^{-1} J^T J$ reduces to the scalar $\det(J)^{-1}$.

## PML coordinate stretch

Inside the PML the real coordinate $x_i$ is analytically continued into the complex plane. The
stretch is Cartesian and axis-aligned, so its Jacobian $J$ is diagonal, $J = \mathrm{diag}(J_{11},
\dots, J_{dd})$, with

!equation
J_{ii} = 1 + \mathrm{i}\, \frac{n\, \alpha}{L_i^{\,n}}\, s_i^{\,n-1}

where $\mathrm{i}$ is the imaginary unit, $s_i \ge 0$ is the depth of the evaluation point into the
PML measured from its inner boundary along axis $i$, and $L_i$ is the PML thickness along that axis
(and side). Outside the PML along a given axis, $s_i = 0$ and $J_{ii} = 1$.

The two input parameters control the profile:

- `decay_coefficient` $\alpha$ sets the overall magnitude of the imaginary (absorbing) part of the
  stretch; it scales $J_{ii}$ linearly. Physically it corresponds to a tuning constant divided by
  the wavenumber.
- `decay_polynomial` $n$ sets how the absorption grows with depth: the imaginary part varies as
  $s_i^{\,n-1}$, normalised by $L_i^{\,n}$ and pre-multiplied by $n$.

Because $J$ is diagonal, $\det(J) = \prod_i J_{ii}$ and the tensor entries are
$(\mathbf{c}_1)_{ii} = a\, J_{ii}^2 / \det(J)$ in three dimensions and $\mathbf{c}_1 = a /
\det(J)$ in two dimensions.

The PML region is this kernel's `block`. Its inner boundary is taken as the bounding box of the
interior (non-PML) region and its outer boundary as the mesh bounding box, so $L_i$ and $s_i$ are
derived from the mesh and no PML thickness needs to be supplied.

## Example Input File Syntax

!listing mfem/pml/cartesian_pml.i block=/Kernels

!syntax parameters /Kernels/MFEMPMLCurlCurlKernel

!syntax inputs /Kernels/MFEMPMLCurlCurlKernel

!syntax children /Kernels/MFEMPMLCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
