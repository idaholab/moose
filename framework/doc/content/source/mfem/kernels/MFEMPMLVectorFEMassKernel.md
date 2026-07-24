# MFEMPMLVectorFEMassKernel

!if! function=hasCapability('mfem')

## Overview

Adds a perfectly-matched-layer (PML) stretched vector FE mass domain integrator for the bilinear
form

!equation
(\mathbf{c}_2 \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ (or $H(\mathrm{div})$) and

!equation
\mathbf{c}_2 = a\, \det(J) (J^T J)^{-1}

is the base scalar `coefficient` $a$ scaled by the complex diagonal tensor arising from the PML
coordinate stretch, with $J$ the Jacobian of the stretch. The stretch, the meaning of the
`decay_coefficient` and `decay_polynomial` parameters, and how the PML geometry is derived from the
mesh are described in [MFEMPMLCurlCurlKernel.md#pml-coordinate-stretch]. Because $J$ is diagonal,
the tensor entries are $(\mathbf{c}_2)_{ii} = a\, \det(J) / J_{ii}^2$.

The PML region is this kernel's `block`.

## Example Input File Syntax

!listing mfem/pml/cartesian_pml.i block=/Kernels

!syntax parameters /Kernels/MFEMPMLVectorFEMassKernel

!syntax inputs /Kernels/MFEMPMLVectorFEMassKernel

!syntax children /Kernels/MFEMPMLVectorFEMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
