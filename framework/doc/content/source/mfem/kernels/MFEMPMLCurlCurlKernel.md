# MFEMPMLCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds a perfectly matched layer (PML) stretched curl-curl domain integrator for the bilinear form

!equation
(\mathbf{c}_{\mathrm{curl}} \vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ and

!equation
\mathbf{c}_{\mathrm{curl}} = a\, \det(J)^{-1} J^T J

is the base scalar `coefficient` $a$ scaled by the complex tensor arising from the PML coordinate
stretch, with $J$ the Jacobian of the stretch (see below). In two dimensions the curl of a vector
field is a scalar, so $\det(J)^{-1} J^T J$ reduces to the scalar $\det(J)^{-1}$.

## PML coordinate stretch

Inside the layer the coordinates are analytically continued into the complex plane. The direction of
that continuation, and the depth into the layer, both come from a harmonic coordinate $\psi$ solved
for once when the kernel is set up:

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

Outside the layer $\psi = 0$,
giving $J = I$ and no stretch.

The two profile parameters control the absorption:

- `decay_coefficient` $\alpha$ is the total imaginary displacement across the layer, so an outgoing
  wave that crosses it, reflects off the outer boundary and returns is attenuated by
  $\exp(-2 k \alpha)$. Physically it corresponds to a tuning constant divided by the wavenumber.
- `decay_polynomial` $n$ sets how the absorption grows with depth. It must exceed one, so that both
  $\vec W$ and its derivative vanish at the inner surface and the layer starts smoothly.

The geometry of the layer is taken entirely from the mesh: the layer is this kernel's `block`, its
inner surface is the set of faces separating that block from the rest of the domain, and its outer
surface is the exterior boundary of the mesh, excluding any boundary that also borders the rest of
the domain and therefore runs alongside the layer rather than capping it. The layer need not be
Cartesian, planar, star shaped or of uniform thickness. Its inner and outer surfaces should be
convex, which guarantees that $\psi$ has no stagnation point at which the stretch direction would be
undefined; this is checked when the kernel is set up.

## Example Input File Syntax

!listing mfem/complex/pml.i block=/Kernels

!syntax parameters /Kernels/MFEMPMLCurlCurlKernel

!syntax inputs /Kernels/MFEMPMLCurlCurlKernel

!syntax children /Kernels/MFEMPMLCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
