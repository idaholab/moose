# MatGradSquareCoupled
!syntax description /Kernels/MatGradSquareCoupled

Implements the term

$$
P |\nabla \psi|^2
$$

where $P$, the prefactor, is a material property, and $\psi$ is a coupled non-linear variable. For a phase-field model coupled with an electrostatic field, the prefactor should be set using a DerivativeParsedMaterial to $P(\eta) = \frac{1}{2} \frac{\partial \epsilon}{\partial \eta}$, where $\epsilon$ is the phase-dependent permittivity and $\eta$ is the order parameter (which is the nonlinear variable for this kernel), and $\psi$ is the electric potential.

!syntax parameters /Kernels/MatGradSquareCoupled

!syntax inputs /Kernels/MatGradSquareCoupled

!syntax children /Kernels/MatGradSquareCoupled
