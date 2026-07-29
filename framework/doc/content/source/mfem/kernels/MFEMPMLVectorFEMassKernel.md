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
stretch, with $J$ the Jacobian of the stretch. The stretch itself, the meaning of the
`decay_coefficient`, `decay_polynomial` and `reference_point` parameters, and how the geometry of
the layer is taken from the mesh are described in
[MFEMPMLCurlCurlKernel.md#pml-coordinate-stretch].

Which of the two PML tensors applies is decided by the quantity a bilinear form integrates rather
than by the operator itself. This kernel integrates the field, so it takes
$\det(J) (J^T J)^{-1}$, whereas [MFEMPMLCurlCurlKernel.md] integrates the curl of the field and
takes $\det(J)^{-1} J^T J$. In the local radial and tangential frame the eigenvalues here are
$\det(J)/J_r^2$ along the ray and $\det(J)/J_t^2$ perpendicular to it.

The layer is this kernel's `block`.

## Example Input File Syntax

!listing mfem/complex/radial_pml.i block=/Kernels

!syntax parameters /Kernels/MFEMPMLVectorFEMassKernel

!syntax inputs /Kernels/MFEMPMLVectorFEMassKernel

!syntax children /Kernels/MFEMPMLVectorFEMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
