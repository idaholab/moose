# MFEMPMLVectorFEMassKernel

!if! function=hasCapability('mfem')

## Overview

Adds a perfectly matched layer (PML) stretched vector FE mass domain integrator for the bilinear
form

!equation
(\mathbf{c}_{\mathrm{field}} \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ (or $H(\mathrm{div})$) and

!equation
\mathbf{c}_{\mathrm{field}} = a\, \det(J) (J^T J)^{-1}

is the base scalar `coefficient` $a$ scaled by the complex tensor arising from the PML coordinate
stretch, with $J$ the Jacobian of the stretch. The stretch itself and the meaning of the
`decay_coefficient` and `decay_polynomial` parameters are described in [MFEMPMLCurlCurlKernel.md#pml-coordinate-stretch].

Which of the two PML tensors applies is decided by the quantity a bilinear form integrates rather
than by the operator itself. This kernel integrates the field, so it takes
$\det(J) (J^T J)^{-1}$, whereas [MFEMPMLCurlCurlKernel.md] integrates the curl of the field and
takes $\det(J)^{-1} J^T J$.

The layer is this kernel's `block`.

## Example Input File Syntax

!listing mfem/complex/pml.i block=/Kernels

!syntax parameters /Kernels/MFEMPMLVectorFEMassKernel

!syntax inputs /Kernels/MFEMPMLVectorFEMassKernel

!syntax children /Kernels/MFEMPMLVectorFEMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
