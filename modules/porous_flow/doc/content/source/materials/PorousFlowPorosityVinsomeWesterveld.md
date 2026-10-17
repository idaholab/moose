# PorousFlowPorosityVinsomeWesterveld

This Material was contributed by Bernard Che Ngu (Baylor University).

When cold fluid is injected into a fractured rock, the rock next to each fracture cools and contracts, and the fracture opens.  This Material models that effect.  It is intended for models in which a fracture is represented by lower-dimensional elements, either in the same mesh as the matrix or in a separate MultiApp (see [flow through fractured media](porous_flow/flow_through_fractured_media.md)), and the porosity of those elements is the fracture aperture (multiplied by the fracture porosity, which is usually unity).  The Material accounts for conductive heat transfer between the fracture and the matrix.  It does not account for fluid flow between the fracture and the matrix, or the heat carried by such flow.

The Material computes porosity (at the nodes or quadrature points, depending on the [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/at_nodes) flag) according to

\begin{equation}
\label{eq:poro_vw}
\phi = \phi_{\mathrm{ref}} + A(P_{f} - P_{f\ \mathrm{ref}}) + B(T - T_{\mathrm{ref}}) + C(\epsilon^{\mathrm{total}}_{ii} - \epsilon^{\mathrm{total}}_{ii\ \mathrm{ref}}) - D\, I = \phi_{\mathrm{linear}} - D\, I \ ,
\end{equation}

where $\phi_{\mathrm{linear}}$ is the porosity given by [PorousFlowPorosityLinear](PorousFlowPorosityLinear.md) (see that page for the meaning of the other symbols), $D$ is the [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/thermal_opening_coeff), and

\begin{equation}
\label{eq:vw_heat_content}
I(t) = \int_{0}^{\infty} \left(T(z, t) - T_{\mathrm{ref}}\right) \mathrm{d}z
\end{equation}

is the heat content (in K$\,$m) of the matrix adjacent to the fracture.  Here $z$ is the distance in the normal direction from the fracture face into the matrix and $T_{\mathrm{ref}}$ is the [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_reference_temperature), which is the undisturbed matrix temperature.  When the matrix cools, $I < 0$, so a positive $D$ increases the porosity (opens the fracture).

The thermal opening depends on how much of the matrix has cooled, not only on the temperature at the fracture face.  The cooled layer is often only centimetres to metres thick, which is much thinner than typical matrix elements, so it is not resolved by the mesh (see also the discussion of "retardation" by the matrix in [flow through fractured media](porous_flow/nomultiapp_flow_through_fractured_media.md)).  Therefore $I$ is computed using a semi-analytic temperature profile, as described below.

## The Vinsome-Westerveld method

Following [!cite](vinsome1980), who developed the method to compute heat losses to cap and base rock in thermal reservoir simulators (including backflow of heat and cyclic temperature changes), the matrix temperature profile normal to the fracture is approximated by

\begin{equation}
\label{eq:vw_profile}
T(z, t) - T_{\mathrm{ref}} = \left(\theta + p z + q z^{2}\right) e^{-z/d}, \qquad d = \frac{\sqrt{\kappa t}}{2} \ ,
\end{equation}

where $\theta = T - T_{\mathrm{ref}}$ is the temperature change at the fracture face, $\kappa$ is the [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_thermal_diffusivity) (thermal conductivity divided by volumetric heat capacity) in the direction normal to the fracture, and $t$ is the time since [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time).  The exponential factor captures the diffusive penetration of the temperature change into the matrix, and $p$ and $q$ adjust the shape of the profile as the history of $\theta$ evolves.  The quantities $\theta$, $p$, $q$ and $d$ depend on $t$ but not on $z$, so all of the $z$-dependence of the profile is shown explicitly in [eq:vw_profile].  With this profile, the heat content is

\begin{equation}
I(t) = \theta d + p d^{2} + 2 q d^{3} \ .
\end{equation}

The coefficients $p$ and $q$ are found at each time step by requiring that (1) the heat-conduction equation $\partial T / \partial t = \kappa\, \partial^{2} T / \partial z^{2}$ holds at the fracture face, and (2) energy is conserved, that is, the change in $I$ over the time step equals the conductive heat flux through the fracture face, $\mathrm{d}I/\mathrm{d}t = -\kappa\, \partial T / \partial z |_{z=0} = \kappa(\theta / d - p)$.  Discretising these with an implicit (backward-Euler) time step of size $\Delta t$ and eliminating $q$ gives

\begin{equation}
\label{eq:vw_update}
p = \frac{a\theta/d + I^{\mathrm{old}} - d^{3}(\theta - \theta^{\mathrm{old}})/a}{3 d^{2} + a}, \qquad
I = I^{\mathrm{old}} + a\left(\frac{\theta}{d} - p\right), \qquad a = \kappa \Delta t \ ,
\end{equation}

where the superscript "old" denotes values at the previous time step.  $\theta$ and $I$ are stored as stateful material properties, and are both zero initially, which means the matrix is at $T_{\mathrm{ref}}$ at the start of the simulation.

Because $I^{\mathrm{old}}$ and $\theta^{\mathrm{old}}$ are fixed during a time step, [eq:vw_update] shows that $I$ is a linear function of the current temperature.  The derivative $\mathrm{d}I/\mathrm{d}T$ is therefore a single number at each node, and the Jacobian contribution of the thermal-opening term is as simple to compute as that of the temperature term in [PorousFlowPorosityLinear](PorousFlowPorosityLinear.md).  [!cite](vinsome1980) note that this linearity makes the method easy to couple implicitly to the temperature, which they found to be stable at large time steps, whereas explicit coupling was not.

## Choosing the thermal-opening coefficient

If the matrix on both sides of the fracture contracts freely with linear thermal expansion coefficient $\alpha_{T}$, each fracture face moves by $\alpha_{T} I$, so

\begin{equation}
D = 2 \alpha_{T} \ .
\end{equation}

If the matrix is instead constrained laterally (zero strain parallel to the fracture, and zero stress normal to it), the normal thermal strain is increased by the factor $(1 + \nu)/(1 - \nu)$, where $\nu$ is the drained Poisson's ratio, so

\begin{equation}
D = 2 \alpha_{T} \frac{1 + \nu}{1 - \nu} \ .
\end{equation}

These values assume the fracture opening is not resisted by the in-situ stress, so they are upper bounds.  In practice, $D$ is often treated as a calibration parameter.  If the porosity of the lower-dimensional elements is the aperture multiplied by a fracture porosity $\phi_{f}$, then $D$ should also be multiplied by $\phi_{f}$.

## Assumptions and limitations

- Heat conduction in the matrix is one-dimensional, normal to the fracture, and the matrix is semi-infinite.  The thermal diffusivity is the value in the direction normal to the fracture (or the matrix is assumed isotropic), and it is constant: it does not depend on temperature or saturation.
- Only conductive heat transfer between the fracture and the matrix is considered.  Fluid flow between the fracture and the matrix is not included in the calculation of $I$.  When the cooled zones of neighbouring fractures overlap, $I$ will be overestimated in magnitude.
- The matrix is at [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_reference_temperature) at [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time).  Before [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time) the thermal-opening term is zero.  Because the diffusion length $d$ is measured from [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time), this should be the time at which the fracture-face temperature first starts to change (for instance, when injection begins), as in [!cite](vinsome1980).
- The profile [eq:vw_profile] is an approximation.  In the example below, for a step change in the fracture-face temperature, the computed $I$ is $3.3\%$ smaller in magnitude than the exact value once the time step is small compared with the elapsed time, and the error is larger when the time step is comparable to the elapsed time (see below).
- The heat lost by the matrix is not removed from the simulation by this Material: the thermal-opening term only modifies porosity.  The heat exchange between the fracture and the matrix is computed by the usual PorousFlow kernels.

## Verification

A step change $\theta = -150\,$K in the fracture-face temperature at $t = 0$ has the exact solution $I = 2\theta\sqrt{\kappa t / \pi}$.  [vw_step_results] compares this with the Vinsome-Westerveld result for $\kappa = 10^{-6}\,$m$^{2}\,$s$^{-1}$, using ten time steps of $3.15\times 10^{6}\,$s (approximately one year in total).  For this step change, the error depends only on the number of time steps taken since the step, that is, on the elapsed time divided by the time step.  It does not depend on $\kappa$ or on the size of the time step itself: the error is $-17.7\%$ after one time step of any size, $-5.1\%$ after ten, and approaches $-3.3\%$ when many time steps are used.

!table id=vw_step_results caption=Matrix heat content for a step change in the fracture-face temperature
| Time (s) | Vinsome-Westerveld $I$ (K$\,$m) | Exact $I$ (K$\,$m) | Difference |
| --- | --- | --- | --- |
| $3.15\times 10^{6}$ | $-247.2$ | $-300.4$ | $-17.7\%$ |
| $1.575\times 10^{7}$ | $-625.0$ | $-671.7$ | $-7.0\%$ |
| $3.15\times 10^{7}$ | $-901.7$ | $-950.0$ | $-5.1\%$ |


The part of the error that comes from the time discretisation decreases approximately linearly with the time step, as shown in [vw_dt_convergence].  The remaining $3.3\%$ is the error of the assumed profile [eq:vw_profile], not of the time discretisation.  For a step change, the continuous form of the method gives $I = \frac{12}{11}\theta\sqrt{\kappa t}$, compared with the exact $I = \frac{2}{\sqrt{\pi}}\theta\sqrt{\kappa t}$, so the ratio is $6\sqrt{\pi}/11 = 0.967$ at all times.

!table id=vw_dt_convergence caption=Error in the matrix heat content at the end of the step-change test (t = 3.15E7 s) as the time step is reduced
| Time step (s) | Vinsome-Westerveld $I$ (K$\,$m) | Difference from exact |
| --- | --- | --- |
| $3.15\times 10^{6}$ | $-901.7$ | $-5.08\%$ |
| $1.575\times 10^{6}$ | $-910.4$ | $-4.16\%$ |
| $7.875\times 10^{5}$ | $-914.6$ | $-3.72\%$ |
| $3.15\times 10^{5}$ | $-916.9$ | $-3.47\%$ |
| $3.15\times 10^{4}$ | $-918.3$ | $-3.34\%$ |
| $3.15\times 10^{3}$ | $-918.4$ | $-3.32\%$ |

The input file for this test is

!listing modules/porous_flow/test/tests/poroperm/vinsome_westerveld.i block=Materials/porosity_VW

Descriptions of other porosity classes can be found in the [porosity documentation](/porous_flow/porosity.md)

!syntax parameters /Materials/PorousFlowPorosityVinsomeWesterveld

!syntax inputs /Materials/PorousFlowPorosityVinsomeWesterveld

!syntax children /Materials/PorousFlowPorosityVinsomeWesterveld
