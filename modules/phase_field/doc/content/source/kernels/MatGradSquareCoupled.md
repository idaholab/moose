# MatGradSquareCoupled

!syntax description /Kernels/MatGradSquareCoupled

Implements the term
\begin{equation}
-P |\nabla \psi|^2,
\end{equation}
where $P$, the prefactor, is a material property, and $\psi$ is a coupled
variable. For a phase-field model coupled with an electrostatic field, the prefactor
should be set using a DerivativeParsedMaterial to
\begin{equation}
P(\eta) = \frac{1}{2} \frac{\partial \epsilon}{\partial \eta},
\end{equation}
where $\epsilon$ is the phase-dependent permittivity and $\eta$ is the order parameter
(which is the nonlinear variable for this kernel), and $\psi$ is the electric potential.

!syntax parameters /Kernels/MatGradSquareCoupled

!syntax inputs /Kernels/MatGradSquareCoupled

!syntax children /Kernels/MatGradSquareCoupled
