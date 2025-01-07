# PorousFlowFullySaturatedMassTimeDerivative

!syntax description /Kernels/PorousFlowFullySaturatedMassTimeDerivative

Consider a fully-saturated, single-phase, single-component fluid in a
THM simulation.  The time-derivative terms from the [fluid governing equation](porous_flow/governing_equations.md) are
\begin{equation}
\frac{\partial}{\partial t} \phi \rho + \phi\rho\dot{\epsilon}_{v} \ ,
\end{equation}
where $\dot{\epsilon}_{v} = \nabla\cdot {\mathbf v}_{s}$ and all other nomenclature is described [here](nomenclature.md).  Using the
[THM evolution of porosity](/porous_flow/porosity.md), along with the
assumption that the fluid bulk modulus, $K_{f}$, and its volumetric
thermal expansion coefficient, $\alpha_{f}$, are constant, and the fluid density is given by
\begin{equation}
\rho = \rho_{0}\exp(P/K_{f} - \alpha_{f} T) \ ,
\end{equation}
the time derivative terms may be written as
\begin{equation}
\label{poromecheq}
\rho \left( \frac{1}{M}\dot{P} + \alpha_{B}\dot{\epsilon}_{v} -
A\dot{T} \right) \ .
\end{equation}
Here $M$ is the so-called Biot Modulus:
\begin{equation}
\label{biotmoddef}
\frac{1}{M} = \frac{\phi}{K_{f}} + \frac{(1 - \alpha_{B})(\alpha_{B} -
  \phi)}{K} \ ,
\end{equation}
and $A$ is an effective volumetric thermal expansion coefficient:
\begin{equation}
\label{constthermexpdef}
A = (\alpha_{B} - \phi)\alpha_{T} + \phi\alpha_{f} \ .
\end{equation}
Notice that disregarding the premultiplication by $\rho$, the above
time-derivative terms in [poromecheq] would be linear in the variables $P$,
displacement, and $T$, if $M$ and $A$ were constant.

In standard poro-mechanics it is usual to calculate $M$ and $A$ at the
initial stage of simulation, and keep them fixed forever afterwards.
Of course this is an approximation since $M$ and $A$ were derived
using the explicit assumption of a porosity that depended on $P$,
$\epsilon_{v}$, and $T$, but it makes finding analytical solutions
much easier.  Therefore, PorousFlow offers the following Materials and
Kernels.  Using these Materials and Kernel allows immediate and precise
comparison with analytical and numerical solutions of poro-mechanics.

1. [PorousFlowConstantBiotModulus](PorousFlowConstantBiotModulus.md) Material, which computes $M$ given by [biotmoddef] during the initial stage of simulation, and keeps it fixed thereafter.  It requires a Porosity Material, but that Material's Property is only used during the initial computation.

2. [PorousFlowConstantThermalExpansionCoefficient](PorousFlowConstantThermalExpansionCoefficient.md) Material, which computes $A$ given by [constthermexpdef] during the initial stage of simulation, and keeps it fixed thereafter.  It requires a Porosity Material, but that Material's Property is only used during the initial computation.

3. The `PorousFlowFullySaturatedMassTimeDerivative` Kernel, which computes one of the following contributions, depending upon the `coupling_type` flag:

- $(\rho) \dot{P}/M$ for fluid-flow-only problems;
- $(\rho)(\dot{P}/M - A\dot{T})$ for TH problems;
- $(\rho)(\dot{P}/M + \alpha_{B}\dot{\epsilon}_{v})$ for HM problems;
- $(\rho)(\dot{P}/M + \alpha_{B}\dot{\epsilon}_{v} - A\dot{T})$ for THM problems.

The `PorousFlowFullySaturatedMassTimeDerivative` Kernel does not employ lumping, which is largely unnecessary in this single-phase, single-component situation.  This means only ``quad-point'' Materials are needed.  In fact, when using all the FullySaturated flow Kernels (see [governing equations](porous_flow/governing_equations.md)) standard Materials evaluated at the quadpoints are needed, which saves on computation time and input-file length.

In each case, the initial pre-multiplication by $\rho$ is optional
(indicated by the parentheses around $\rho$).

When the Kernel is pre-multiplied by $\rho$, which is the default,
it is computing the time derivative of fluid mass.  This allows
the Kernel to be easily used with the remainder of PorousFlow: the
BCs, the Postprocessors, the AuxKernels, and the DiracKernels are all
based on mass.

When the pre-multiplication is not performed, this Kernel is computing the
time derivative of fluid volume.  This has two great advantages:

- the time-derivatives are linearised, resulting in excellent nonlinear convergence;
- comparing with results from poro-mechanics theory is straightforward.

However, this means additional care must be taken.

- The flow Kernel [PorousFlowFullySaturatedDarcyBase](PorousFlowFullySaturatedDarcyBase.md) should also not pre-multiply by density.
- The BCs, Postprocessors, AuxKernels, and DiracKernels must be written in such a way to operate in the fluid-volume scenario rather than the default fluid-mass scenario.
- The flag `consistent_with_displaced_mesh` should be set `false` in the [PorousFlowVolumetricStrain](PorousFlowVolumetricStrain.md) Material.




!syntax parameters /Kernels/PorousFlowFullySaturatedMassTimeDerivative

!syntax inputs /Kernels/PorousFlowFullySaturatedMassTimeDerivative

!syntax children /Kernels/PorousFlowFullySaturatedMassTimeDerivative

!bibtex bibliography
