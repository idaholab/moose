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

## Finite matrix thickness

By default the matrix is semi-infinite, so $|I|$ keeps growing (as $\sqrt{t}$ for a constant fracture-face temperature).  If [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_thickness) $L$ is given, the matrix is a layer $0 \le z \le L$ whose far side is insulated, which represents the rock between parallel fractures a distance $2L$ apart.  The profile [eq:vw_profile] is then replaced by

\begin{equation}
\label{eq:vw_finite}
T(z, t) - T_{\mathrm{ref}} = P(z)\, e^{-z/d} + P(2L - z)\, e^{-(2L - z)/d}, \qquad P(z) = A + Bz + Cz^{2} \ ,
\end{equation}

which is symmetric about $z = L$, so $\partial T / \partial z = 0$ there.  At each time step, $A$, $B$, $C$ and the heat content of the layer, $I = \int_{0}^{L} (T - T_{\mathrm{ref}})\, \mathrm{d}z = \int_{0}^{2L} P(z)\, e^{-z/d}\, \mathrm{d}z$, are found from the fracture-face temperature, the heat-conduction equation at $z = 0$ and the energy balance $\mathrm{d}I/\mathrm{d}t = -\kappa\, \partial T / \partial z |_{z=0}$, as in [eq:vw_update].  These conditions are linear, so $I$ is again a linear function of the current temperature.  As $L \to \infty$ the image term vanishes and the method reduces exactly to [eq:vw_update].  Because the layer is insulated, heat entering through the fracture face stays in the layer, and $I \to \theta L$ once the whole layer has reached the fracture-face temperature, so the opening is bounded.

A finite thickness bounds the opening, and it can also change how the opening responds to a fluctuating fracture-face temperature.  [vw_finite_examples] shows two examples, with $\kappa = 1$ and $L = 1$.  It plots $-I$, which is proportional to the change in porosity (aperture), because cooling opens the fracture when $D > 0$.

- $\theta = \sin\sqrt{t}$ (top).  At small $t$ the semi-infinite and finite-thickness results agree.  Once the cycles are slow compared with $L^{2}/\kappa$, the finite-thickness heat content stays close to $\theta L$, so the aperture cycles with the fracture-face temperature.  With a semi-infinite matrix, $I$ integrates the whole temperature history, so its swings grow and lag behind the temperature: at $t = 100$, for instance, $\theta < 0$ but $I > 0$.
- $\theta = \sin\left(2\pi(16 - t)^{1/4}\right)$ (bottom).  The long cold pulse takes the finite-thickness heat content to $I = -L$ (the whole layer at $\theta = -1$), and the following warm phase reverses it to about $I = +0.95L$.  With a semi-infinite matrix, $I$ stays negative, because it is dominated by the deep cooled rock.

In both examples the finite-thickness result is within $0.01$ of the exact solution for an insulated layer, obtained from the series solution for a slab and Duhamel's theorem.  The semi-infinite result differs from its own exact solution by up to about $20\%$ of its amplitude under these slow cycles, which is a property of the single-profile approximation.

!media large_media/porous_flow/vinsome_westerveld_finite_thickness.png
       id=vw_finite_examples
       style=width:90%;margin:auto;
       caption=Heat content of the matrix ($-I$, proportional to the change in aperture) for two fluctuating fracture-face temperatures, computed with a semi-infinite matrix and with an insulated layer of thickness $L = 1$, compared with the exact solutions.

The input files for these examples are [vinsome_westerveld_sqrt.i](modules/porous_flow/test/tests/poroperm/vinsome_westerveld_sqrt.i) and [vinsome_westerveld_pulse.i](modules/porous_flow/test/tests/poroperm/vinsome_westerveld_pulse.i).

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

- Heat conduction in the matrix is one-dimensional, normal to the fracture, and the matrix is semi-infinite.  Alternatively, [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_thickness) makes the matrix a layer of finite thickness with an insulated far side (see [eq:vw_finite]).  The thermal diffusivity is the value in the direction normal to the fracture (or the matrix is assumed isotropic), and it is constant: it does not depend on temperature or saturation.
- Only conductive heat transfer between the fracture and the matrix is considered.  Fluid flow between the fracture and the matrix is not included in the calculation of $I$.  When the cooled zones of neighbouring fractures overlap, $I$ will be overestimated in magnitude.
- The matrix is at [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_reference_temperature) at [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time).  Before [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time) the thermal-opening term is zero, and any difference between the fracture-face temperature and [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/matrix_reference_temperature) at [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time) is treated as a step change at that time.  Because the diffusion length $d$ is measured from [!param](/Materials/PorousFlowPorosityVinsomeWesterveld/vw_start_time), this should be the time at which the fracture-face temperature first starts to change (for instance, when injection begins), as in [!cite](vinsome1980).
- The profile [eq:vw_profile] is an approximation.  In the example below, for a step change in the fracture-face temperature, the computed $I$ is $3.3\%$ smaller in magnitude than the exact value once the time step is small compared with the elapsed time, and the error is larger when the time step is comparable to the elapsed time (see below).
- The heat lost by the matrix is not removed from the simulation by this Material: the thermal-opening term only modifies porosity.  The heat exchange between the fracture and the matrix is computed by the usual PorousFlow kernels.

## Verification

For a step change $\theta$ in the fracture-face temperature at $t = 0$, the exact matrix temperature is $T - T_{\mathrm{ref}} = \theta\,\mathrm{erfc}\left(z / 2\sqrt{\kappa t}\right)$, and integrating over $z$ gives $I(t) = \frac{2}{\sqrt{\pi}}\theta\sqrt{\kappa t}$.  The Vinsome-Westerveld equations can also be solved exactly in this case, if the time step is infinitesimally small.  Writing $p = \alpha\theta/d$ and $q = \beta\theta/d^{2}$, the heat-conduction equation at $z = 0$ (where $\partial T/\partial t = 0$ because $\theta$ is constant) gives $1 - 2\alpha + 2\beta = 0$, so $I = 3\alpha\theta d$.  The energy balance $\mathrm{d}I/\mathrm{d}t = \kappa(\theta/d - p)$, with $d = \sqrt{\kappa t}/2$, then gives $\alpha = 8/11$ and $I(t) = \frac{12}{11}\theta\sqrt{\kappa t}$.  The ratio of the two is $6\sqrt{\pi}/11 = 0.967$, so the assumed profile underestimates the magnitude of $I$ by $3.3\%$ at all times.

With a finite time step there is an additional error.  For a step change, this depends only on the number of time steps since the step ($t/\Delta t$), not on $\kappa$ or on the size of $\Delta t$, as shown in [vw_step_results].

!table id=vw_step_results caption=Error in the matrix heat content for a step change in the fracture-face temperature
| Number of time steps since the step | 1 | 10 | 20 | 40 | 100 | 1000 | 10000 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Error in $I$ | $-17.7\%$ | $-5.08\%$ | $-4.16\%$ | $-3.72\%$ | $-3.47\%$ | $-3.34\%$ | $-3.32\%$ |

The input file for the step-change test ($\theta = -150\,$K, $\kappa = 10^{-6}\,$m$^{2}\,$s$^{-1}$) is

!listing modules/porous_flow/test/tests/poroperm/vinsome_westerveld.i block=Materials/porosity_VW

Descriptions of other porosity classes can be found in the [porosity documentation](/porous_flow/porosity.md)

!syntax parameters /Materials/PorousFlowPorosityVinsomeWesterveld

!syntax inputs /Materials/PorousFlowPorosityVinsomeWesterveld

!syntax children /Materials/PorousFlowPorosityVinsomeWesterveld
