# MFEMPerfectlyMatchedLayerFunction

!if! function=hasCapability('mfem')

## Overview

Declares an MFEM coefficient that applies the complex coordinate stretch of a perfectly matched
layer (PML) to the base scalar `coefficient` $a$ of a bilinear form. The resulting coefficient is
passed to [MFEMCurlCurlKernel.md] or [MFEMVectorFEMassKernel.md] acting on the layer, in place of
the plain coefficient of that form.

Which stretch tensor applies is set by the quantity the bilinear form integrates rather than by the
operator using it. Pulling the weak form back from stretched to physical coordinates gives one
factor for an integrand holding the curl of the field and another for one holding the field itself,
selected by `tensor`:

!equation
\mathbf{c}_{\mathrm{curl}} = a\, \det(J)^{-1} J^T J, \qquad
\mathbf{c}_{\mathrm{field}} = a\, \det(J) (J^T J)^{-1}

with $J$ the Jacobian of the stretch. In two dimensions the curl of a vector field is a scalar, so
$\mathbf{c}_{\mathrm{curl}}$ reduces to the scalar $a \det(J)^{-1}$; set
`coefficient_type = scalar` there, and `coefficient_type = matrix` in every other case.

The stretch is complex symmetric rather than Hermitian, and the complex system is assembled from two
real bilinear forms, so `component` selects the real or the imaginary part. One function is declared
for each combination of `coefficient_type`, `tensor` and `component` that the problem needs; those
sharing a layer and a profile solve for the harmonic coordinate below only once between them.

## PML coordinate stretch

Inside the layer the coordinates are analytically continued into the complex plane. The direction of
that continuation, and the depth into the layer, both come from a harmonic coordinate $\psi$ solved
for once when the function is set up:

!equation
\nabla^2 \psi = 0 \,\, \mathrm{in} \,\, \Omega_{\mathrm{PML}}, \qquad
\psi = 0 \,\, \mathrm{on} \,\, \Gamma_{\mathrm{in}}, \qquad
\psi = 1 \,\, \mathrm{on} \,\, \Gamma_{\mathrm{out}}

with no flux through any remaining lateral wall. The level sets of $\psi$ foliate the layer whatever
its shape, interpolating smoothly between the shape of its inner surface and that of its outer
surface, and

!equation
\hat n = \frac{\nabla \psi}{|\nabla \psi|}

is their unit normal. Each point of the layer is then displaced into the complex plane along that
normal,

!equation
\tilde{\vec x} = \vec x + \mathrm{i}\, \vec W, \qquad
\vec W = \alpha\, \psi^{\,n}\, \hat n, \qquad
J = I + \mathrm{i} \nabla \vec W

Outside the layer $\psi = 0$, giving $J = I$ and no stretch.

The two profile parameters control the absorption:

- `decay_coefficient` $\alpha$ is the total imaginary displacement across the layer, so an outgoing
  wave that crosses it, reflects off the outer boundary and returns is attenuated by
  $\exp(-2 k \alpha)$. Physically it corresponds to a tuning constant divided by the wavenumber.
- `decay_polynomial` $n$ sets how the absorption grows with depth. It must exceed one, so that both
  $\vec W$ and its derivative vanish at the inner surface and the layer starts smoothly.

The geometry of the layer is taken entirely from the mesh: the layer is this function's `block`, its
inner surface is the set of faces separating that block from the rest of the domain, and its outer
surface is the exterior boundary of the mesh, excluding any boundary that also borders the rest of
the domain and therefore runs alongside the layer rather than capping it. The layer need not be
Cartesian, planar, star shaped or of uniform thickness. Its inner and outer surfaces should be
convex, which guarantees that $\psi$ has no stagnation point at which the stretch direction would be
undefined; a coarse check for this runs when the function is set up.

## Example Input File Syntax

!listing mfem/complex/pml.i block=/Functions

!listing mfem/complex/pml.i block=/Kernels

!syntax parameters /Functions/MFEMPerfectlyMatchedLayerFunction

!syntax inputs /Functions/MFEMPerfectlyMatchedLayerFunction

!syntax children /Functions/MFEMPerfectlyMatchedLayerFunction

!if-end!

!else
!include mfem/mfem_warning.md
