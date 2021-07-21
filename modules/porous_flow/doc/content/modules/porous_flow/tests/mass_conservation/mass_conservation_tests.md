# Mass computation and conservation

The total fluid mass of species ${\mathrm{sp}}$ within a volume $V$ is
\begin{equation}
\int_{V} \phi\sum_{{\mathrm{ph}}}\rho_{{\mathrm{ph}}} S_{{\mathrm{ph}}}\chi_{{\mathrm{ph}}}^{{\mathrm{sp}}} \ .
\label{eq:mass}
\end{equation}
It must be checked that MOOSE calculates this correctly, using the [PorousFlowFluidMass postprocessor](PorousFlowFluidMass.md), in order that mass-balances be correct, and also because this quantity is used in a number of other tests.

## Single-phase, single-component fluid

A 1D model with $-1\leq x \leq 1$, and with three elements of size 1 is
created with the following properties:

!table id=single_phase_1_comp caption=Parameter values in the single-phase, single-component test
| Parameter | Value |
| --- | --- |
| Constant fluid bulk modulus | $1\,$Pa |
| Fluid density at zero pressure | $1\,$kg.m$^{-3}$ |
| Van Genuchten $m$ | 0.5 |
| Van Genuchten $\alpha$ | $1\,$Pa$^{-1}$ |
| Porosity | 0.1 |

The porepressure is set at $P=x$.

!listing modules/porous_flow/test/tests/mass_conservation/mass01.i

Recall that in PorousFlow, mass is lumped to the nodes.  Therefore,
the integral above is evaluated at the nodes, and a sum of the results
is outputted as the PorousFlowFluidMass postprocessor.
Using the properties given above, this yields:

!table id=nodal_masses_11 caption=Nodal masses in the single-phase, single-component test
| $x$ | $p$ | Density | Saturation | Nodal mass |
| --- | --- | --- | --- | --- |
|-1 | -1 | 0.367879441 | 0.707106781 | 0.008671002 |
| -0.333333333 | -0.333333333 | 0.716531311 | 0.948683298 | 0.02265871 |
| -0.333333333 | -0.333333333 | 0.716531311 | 0.948683298 | 0.02265871 |
| 0.333333333 | 0.333333333 | 1.395612425 | 1 | 0.046520414 |
| 0.333333333 | 0.333333333 | 1.395612425 | 1 | 0.046520414 |
| 1| 1 | 2.718281828 | 1 | 0.090609394 |
| | | | Total | 0.237638643 |

MOOSE also gives the total mass as 0.237638643\,kg.

## Single-phase, two-components

This is similar to the previous section, but has two fluid ocmponents
components.  The mass fraction is fixed at
\begin{equation}
\chi_{{\mathrm{ph}}=0}^{{\mathrm{sp}}=0} = x^{2} \ .
\end{equation}

!listing modules/porous_flow/test/tests/mass_conservation/mass02.i

!table id=nodal_masses_12 caption=Nodal masses in the single-phase, 2-component test
| $x$ | $p$ | Density | Saturation | $\chi_{{\mathrm{ph}}=0}^{{\mathrm{sp}}=0}$ | Nodal mass$_{{\mathrm{sp}}=0}$ | Nodal mass$_{{\mathrm{sp}}=1}$ |
| --- | --- | --- | --- | --- | --- | --- |
| -1 | -1 | 0.367879441 | 0.707106781 | 1 | 0.008671 | 0 |
| -0.333333333 | -0.333333333 | 0.716531311 | 0.948683298 | 0.111111 | 0.00251763 | 0.02014108 |
| -0.333333333 | -0.333333333 | 0.716531311 | 0.948683298 | 0.111111 | 0.00251763 | 0.02014108 |
| 0.333333333 | 0.333333333 | 1.395612425 | 1 | 0.111111 | 0.00516893 | 0.04135148 |
| 0.333333333 | 0.333333333 | 1.395612425 | 1 | 0.111111 | 0.00516893 | 0.04135148 |
| 1| 1 | 2.718281828 | 1 | 1 | 0.09060939 | 0 |
| | | | | Total | 0.11465353 | 0.12298511 |

MOOSE produces the expected answer.

## Two-phase, two-components

A 1D model with two elements from $0 \leq x \leq 1$ is created, with two phases (0 and 1), and two fluid components (0 and 1). The phase densities are calculated using a constant bulk modulus fluid with bulk modulus of 1 Pa. The density at zero pressure is 1 kg.m${-3}$ for phase 0, and 0.1 kg.m$^{-3}$ for phase 1. The porepressure of phase 0 is held fixed at 1 Pa, and a constant capillary pressure of 0 is specified so that the pressure of phase 1 is also 1 Pa. This results in phase densities of $e = 2.71812818...$ kg.m$^{-3}$ for phase 0, and $0.271812818...$ kg.m$^{-3}$ for phase 1.

Saturation of phase 1 varies linearly as $1 - x$, while porosity is 0.1 throughout. The mass fraction of species 0 in fluid phase 0 is specified as 0.3, while the mass fraction of species 0 in phase 1 is 0.55.

!listing modules/porous_flow/test/tests/mass_conservation/mass05.i

It is simple to calculate the total mass of each component in each phase using [eq:mass], the results of which are

!table id=nodal_masses_22 caption=Masses in the 2-phase, 2-component test
| Species | Phase | Total mass [eq:mass] | Total mass (MOOSE) |
| --- | --- | --- | --- |
0 | 0 | 0.04077423 | 0.04077423 |
0 | 1 | 0.007475275 | 0.007475275 |
0 | all | 0.04824950 |  0.04824950 |
1 | 0 | 0.09513986 | 0.09513986 |
1 | 1 | 0.006116134 | 0.006116134 |
1 | all | 0.10125560 | 0.1012560 |


## Constant fluid source

A fluid source of $0.1\,$kg.m$^{-3}$.s$^{-1}$ is introduced into a single element, and the [PorousFlowFluidMass](PorousFlowFluidMass.md) postprocessor is used to record the fluid mass as a function of time:

!listing modules/porous_flow/test/tests/mass_conservation/mass03.i

## Mass conservation in a deforming material

A single unit element, with roller BCs on its sides and bottom, is compressed at a uniform rate:
\begin{equation}
u_{z} = -0.01 t \ .
\end{equation}
Here $u_{z}$ is the vertical displacement and $t$ is time.  There is no fluid flow and the material's boundaries are impermeable.  Fluid mass conservation is checked.

!listing modules/porous_flow/test/tests/mass_conservation/mass04.i

Note the `use_displaced_mesh = true` option in the [PorousFlowFluidMass postprocessor](PorousFlowFluidMass.md): it is necessary to correctly compute the mass.

Under these conditions
\begin{equation}
\begin{array}{rcl}
P & = & P_{0} - K_{f}\mathrm{log}(1 - u_{z}) \ , \\
\sigma_{xx}^{\mathrm{eff}} & = & (K - \frac{2}{3}G)u_{z}/L \ , \\
\sigma_{zz}^{\mathrm{eff}} & = & (K + \frac{2}{3}G)u_{z}/L \ , \\
\end{array}
\end{equation}
Here $P$ is the fluid porepressure, which is $P_{0}$ at $t=0$; $K_{f}$ is the fluid bulk modulus; $\sigma^{\mathrm{eff}}$ is the effective stress; $K$ is the drained bulk modulus of the material; $G$ is the shear modulus of the material; and $L$ is the height of the sample.

PorousFlow produces these results exactly, and, importantly, conserves fluid mass.

Similar tests are run in "RZ" coordinates.

## Mass conservation in a deformable material with a source

A single unit element, with roller BCs on its sides and bottom, is injected with fluid at rate $s$ kg.s$^{-1}$.  Its top is free to move.  

!listing modules/porous_flow/test/tests/mass_conservation/mass11.i

Under these conditions the fluid mass should increase at rate $s$, and the porepressure should increase accordingly.  The total stress $\sigma_{zz}$ should be zero since the top is free to move, so the effective stress should be $\sigma_{zz} = \alpha_{B} P$.  MOOSE produces these results exactly.

Similar tests are run in "RZ" coordinates.


## Mass computation with a saturation threshold in multi-component, multi-phase fluids

The [PorousFlowFluidMass postprocessor](PorousFlowFluidMass.md) may be used to compute the total mass of each component in each phase, as well as the total mass of each component in all phases.  Furthermore, a saturation threshold may be set to  only count the fluid above the threshold.

!listing modules/porous_flow/test/tests/mass_conservation/mass06.i

