# CHInterface
!syntax description /Kernels/CHInterface

Implements the Cahn-Hilliard term for the $\frac{\kappa_i}2|\nabla c_i|^2$ gradient
energy contribution for the isotropic mobility case. Its weak form is

$$
\left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right),
$$

where $\kappa_i$ (`kappa_name`) is the gradient energy coefficent, $c_i$ the conserved
non-linear order parameter variable the kernel is acting on, $M_i$ (`mob_name`) is
the scalar (isotropic) mobility associated with the order parameter, and $\psi_m$
is the test function.

!syntax parameters /Kernels/CHInterface

!syntax inputs /Kernels/CHInterface

!syntax children /Kernels/CHInterface
