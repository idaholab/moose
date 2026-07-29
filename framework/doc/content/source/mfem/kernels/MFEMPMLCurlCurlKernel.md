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

Inside the layer the radial coordinate is analytically continued into the complex plane. Depth is
measured along the straight line from a reference point $\vec p_0$ through the evaluation point
$\vec x$. That ray meets the inner surface of the layer at a distance $r_{\mathrm{in}}$ from
$\vec p_0$ and the outer surface of the mesh at $r_{\mathrm{out}}$, so with $r = |\vec x - \vec p_0|$

!equation
L = r_{\mathrm{out}} - r_{\mathrm{in}}, \qquad s = r - r_{\mathrm{in}}

are the thickness of the layer and the depth into it along that ray. Writing the stretched radius as
$\tilde r = r + \mathrm{i}\, g(s)$, the stretch is diagonal in the local radial and tangential frame,

!equation
g(s) = \frac{\alpha\, s^{\,n}}{L^{\,n}}, \qquad
J_r = 1 + \mathrm{i}\, \frac{n\, \alpha}{L^{\,n}}\, s^{\,n-1}, \qquad
J_t = 1 + \mathrm{i}\, \frac{g(s)}{r}

where $J_r = \mathrm{d}\tilde r/\mathrm{d}r$ acts along the ray and $J_t = \tilde r / r$ acts
perpendicular to it. In Cartesian coordinates this is the full symmetric tensor

!equation
J = J_r\, \hat r \otimes \hat r + J_t\, (I - \hat r \otimes \hat r), \qquad
\det(J) = J_r\, J_t^{\,d-1}

for $d$ spatial dimensions, with $\hat r$ the unit vector from $\vec p_0$ towards $\vec x$. Outside
the layer $s \le 0$, giving $J_r = J_t = 1$ and no stretch.

The two profile parameters control the absorption:

- `decay_coefficient` $\alpha$ sets the overall magnitude of the imaginary (absorbing) part of the
  stretch; it scales the imaginary parts of $J_r$ and $J_t$ linearly. Physically it corresponds to a
  tuning constant divided by the wavenumber.
- `decay_polynomial` $n$ sets how the absorption grows with depth: the imaginary part of $J_r$
  varies as $s^{\,n-1}$, normalised by $L^{\,n}$ and pre-multiplied by $n$.

Because the stretch is measured along rays rather than along the coordinate axes, the layer need not
be Cartesian or planar. Its geometry is taken from the mesh: the layer is this kernel's `block`, its
inner surface is the set of faces separating that block from the rest of the domain, and its outer
surface is the exterior boundary of the mesh. No layer thickness has to be supplied.

The reference point is set with `reference_point` and defaults to the barycenter of the mesh. It
must lie inside the mesh and outside the layer, and both surfaces must be star shaped about it, so
that every ray crosses the inner surface before the outer one. These conditions are checked when the
kernel is set up.

## Example Input File Syntax

!listing mfem/complex/radial_pml.i block=/Kernels

!syntax parameters /Kernels/MFEMPMLCurlCurlKernel

!syntax inputs /Kernels/MFEMPMLCurlCurlKernel

!syntax children /Kernels/MFEMPMLCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
