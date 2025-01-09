# Heat energy produced by plastic deformation

In [PorousFlow](porous_flow/governing_equations.md), it is assumed that plastic deformation causes a heat energy-density rate
(J.m$^{-3}$.s$^{-1}$) of
\begin{equation}
c (1-\phi) \sigma_{ij}\epsilon^{\mathrm{plastic}}_{ij} \ ,
\end{equation}
where $c$ is a coefficient (s$^{-1}$), $\phi$ is the porosity,
$\sigma$ is the stress, and $\epsilon^{\mathrm{plastic}}$ is the
plastic strain.  This is implemented in the [PorousFlowPlasticHeatEnergy Kernel](PorousFlowPlasticHeatEnergy.md)

In the set of simple tests described below, the porous skeleton contains no fluid, and no heat flow is considered, so the heat energy
released simply heats up the porous skeleton:
\begin{equation}
\frac{\partial}{\partial t} (1 - \phi)c_{R}\rho_{R} T = c (1-\phi)
\sigma_{ij}\epsilon^{\mathrm{plastic}}_{ij} \ .
\end{equation}
The porosity ($\phi$) and volumetric heat
capacity of the rock grains ($c_{R}\rho_{R}$) are chosen to be constant.

Perfect, capped, weak-plane plasticity is used, so that the admissible
zone is defined by
\begin{equation}
\begin{array}{rcl}
\sigma_{zz} & \leq & S_{T} \ , \\
\sigma_{zz} & \geq & -S_{C} \, \\
\sqrt{\sigma_{zx}^{2} + \sigma_{zy}^{2}} + \sigma_{zz}\tan\Phi & \leq
& C \ .
\end{array}
\end{equation}
Here $S_{T}$ is the tensile strength, $S_{C}$ is the compressive
strength, $C$ is the cohesion and $\Phi$ is the friction angle.  The
elastic tensor is chosen to be isotropic
\begin{equation}
E_{ijkl} = \lambda \delta_{ij}\delta_{kl} + \mu
(\delta_{ik}\delta_{jl} + \delta_{il}\delta_{jk}) \ .
\end{equation}
The parameters in these expressions are chosen to be: $S_{T}=1$,
$S_{C}=1$, $C=1$, $\Phi=\pi/4$, $\lambda=1/2$, and $\mu=1/4$ (all in
consistent units).

In each experiment a single finite-element of unit size is used.

## Tensile failure

The top of the finite element is pulled upwards with displacement:
\begin{equation}
u_{z} = t \ \ \ \mathrm{for }\ \ z=1\ ,
\end{equation}
while the displacement on the element's bottom, and in the $x$ and $y$ directions is chosen to be
zero.

!listing modules/porous_flow/test/tests/plastic_heating/tensile01.i

This implies the only non-zero component of total strain is
\begin{equation}
\epsilon^{\mathrm{total}}_{zz} = t \ .
\end{equation}
The constitutive law implies
\begin{equation}
\sigma_{zz} = t \ .
\end{equation}
This stress is admissible for $t\leq 1$, while for $t>1$ the system
yields in tension:
\begin{equation}
\sigma_{zz} = 1 \ \ \ \mathrm{for }\ \  t>1 \ ,
\end{equation}
and the plastic strain is,
\begin{equation}
\epsilon^{\mathrm{plastic}}_{zz} = t - 1 \ \ \ \mathrm{for }\ \  t>1 \ .
\end{equation}
This means that the material's temperature should increase as
\begin{equation}
c_{R}\rho_{R}\dot{T} = c \ \ \ \mathrm{for } t>1\ \  \ ,
\end{equation}
with no temperature increase for $t\leq 1$.

This result is reproduced exactly by PorousFlow.

## Compressive failure

The top of the finite element is pushed downwards with displacement:
\begin{equation}
u_{z} = -t \ \ \ \mathrm{for }\ \ z=1\ ,
\end{equation}
while the displacement on the element's bottom, and in the $x$ and $y$ directions is chosen to be
zero.

!listing modules/porous_flow/test/tests/plastic_heating/compressive01.i

This implies only non-zero component of total strain is
\begin{equation}
\epsilon^{\mathrm{total}}_{zz} = -t \ .
\end{equation}
The constitutive law implies
\begin{equation}
\sigma_{zz} = -t \ .
\end{equation}
This stress is admissible for $t\leq 1$, while for $t>1$ the system
yields in compression
\begin{equation}
\sigma_{zz} = -1 \ \ \ \mathrm{for }\ \  t>1 \ ,
\end{equation}
and the plastic strain is,
\begin{equation}
\epsilon^{\mathrm{plastic}}_{zz} = -(t - 1) \ \ \ \mathrm{for }\ \  t>1 \ .
\end{equation}
This means that the material's temperature should increase as
\begin{equation}
c_{R}\rho_{R}\dot{T} = c \ \ \ \mathrm{for } t>1\ \  \ ,
\end{equation}
and there should be no temperature increase for $t\leq 1$.

MOOSE produces this result exactly

## Shear failure

The top of the finite element is sheared with displacement:
\begin{equation}
u_{x} = t \ \ \ \mathrm{for }\ \ z=1\ ,
\end{equation}
while the displacement on the element's bottom, and in the $y$ and $z$ directions is chosen to be
zero.

!listing modules/porous_flow/test/tests/plastic_heating/shear01.i

This implies only non-zero component of total strain is
\begin{equation}
\epsilon^{\mathrm{total}}_{xz} = t \ .
\end{equation}
The constitutive law implies
\begin{equation}
\sigma_{xz} = t/4 \ .
\end{equation}
This stress is admissible for $t\leq 4$, while for $t>4$ the system
yields in shear:
\begin{equation}
\sigma_{xz} = 1 \ \ \ \mathrm{for }\ \  t>4 \ ,
\end{equation}
and the plastic strain is,
\begin{equation}
\epsilon^{\mathrm{plastic}}_{xz} = t - 4 \ \ \ \mathrm{for }\ \  t>4 \ .
\end{equation}
This means that the material's temperature should increase as
\begin{equation}
c_{R}\rho_{R}\dot{T} = c \ \ \ \mathrm{for }\ \  t>4 \ ,
\end{equation}
while the right-hand side is zero for $t\leq 4$.

PorousFlow produces this result exactly.
