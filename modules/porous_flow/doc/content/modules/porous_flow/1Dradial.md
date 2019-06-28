# Radial flow from an injection well

This problem was presented in [!cite](pruess2002) and [!cite](pruess2004) as intercomparison problem 3. It serves
as a useful test as injection into a one dimensional radial reservoir admits a similarity solution
$\zeta = r^2/t$, where $r$ is the radial distance from the injection well, and $t$ is time. This similarity solution holds even when complicated physics, such as mutual solubility of fluid components, is included.

## Model

### Reservoir properties

The properties of the reservoir used in the model are summarized in [tab:res].

!table id=tab:res caption=Reservoir properties
| Property |  Value |
| - | - |
| Pressure | 12 MPa |
| Temperature | 45 $^{\circ}$C |
| Permeability | $10^{-13}$ m$^2$ (100 md) |
| Porosity | 0.12 |
| NaCl mass fraction | 0.15 |

Relative permeability of the aqueous phase is represented by a [van Genuchten](/PorousFlowRelativePermeabilityVG.md) model, with exponent $m = 0.457$ and residual saturation
$S_{r} = 0.3$. Gas phase relative permeability is represented using a [Corey](/PorousFlowRelativePermeabilityCorey.md) model, with exponent $n = 2$ and residual saturation $S_r = 0.05$. Capillary pressure is given by a [van Genuchten](/PorousFlowCapillaryPressureVG.md) model, again with exponent $m = 0.457$, but this time with zero residual saturation.

Unlike other simulators used in the intercomparison study, PorousFlow is able to model this problem using a
one-dimensional mesh that is rotated about the vertical axis to represent a true 1D radial model. TOUGH2, in contrast, uses a 2D mesh rotated about the vertical axis, which is why the model presented in [!cite](pruess2004) has a height of 100m. For this problem, we use a 1D mesh, and reduce the rate of injection accordingly so that 1 kg/s of CO$_2$ is injected at the origin.

## Results

[fig:satprofile] shows the evolution of the saturation front of the CO$_2$-rich gas phase in this problem. It is interesting to observe the presence of a fully gas saturated region near the injection point that emerges after sufficient time. In the model, the residual saturation of the liquid phase is set to 0.3. For liquid saturations below this, the relative permeability is zero, and no advection of the liquid phase is possible. However, in this miscible model, the brine below residual saturation is slowly evaporated into the gas phase as it is injected, whereby the liquid fluid component is advected as part of the gas phase. This drying out process continues until all of the aqueous phase is removed.

!media media/porous_flow/problem3_saturation.png
       id=fig:satprofile
       style=width:60%;margin-left:10px;
       caption=Gas saturation profile at various times

As previously stated, this one-dimensional radial model admits a similarity solution. [fig:satsim] shows a comparison of the four profiles presented in [fig:satprofile] as a function of the similarity variable $\zeta$. As this figure shows, all four profiles are essentially coincident. As discussed in [!cite](pruess2002), three distinct regions can be observed. For $\zeta < 10^{-5}$, no aqueous phase is present (note that some liquid fluid component can be present in the gas phase, as discussed earlier). This region corresponds to small radial distances and long times. For $10^{-5} < \zeta < 10^{-2}$, both gas and liquid phases coexist, while for $\zeta > 10^{-2}$, only a liquid phase is present (the gas phase hasn't reached this radial distance yet).

!media media/porous_flow/problem3_saturation_similarity.png
      id=fig:satsim
      style=width:60%;margin-left:10px;
      caption=Gas saturation profile vs similarity variable

The similarity solution can also be verified by comparing the spatial results at a fixed time with the temporal results at a fixed radial distance. Examples of this are presented in [fig:xco2sim] and [fig:yh2osim] for the mass fraction of CO$_2$ in the liquid phase (dissolved) and the mass fraction of H$_2$O in the gas phase (evaporation). In the two-phase region $10^{-5} < \zeta < 10^{-2}$, each of these mass fractions are given by their equilibrium value. Outside the two-phase region, these mass fractions are zero, as expected. We observe good agreement between the two results (spatial profile and temporal profile), which provides further evidence of the presence of a similarity solution (some minor oscillation is observed in the temporal profile due to the finite grid size).

!media media/porous_flow/problem3_xco2_similarity.png
      id=fig:xco2sim
      style=width:60%;margin-left:10px;
      caption=Mass fraction of CO$_2$ in the liquid phase vs similarity variable

!media media/porous_flow/problem3_yh2o_similarity.png
      id=fig:yh2osim
      style=width:60%;margin-left:10px;
      caption=Mass fraction of H$_2$O in the liquid phase vs similarity variable

Note that these results are slightly different to the results presented in [!cite](pruess2002) and [!cite](pruess2004). This is because PorousFlow features an updated equation of state which was not available when the intercomparison results were first published. However, they do exhibit excellent agreement with the results presented in [!cite](pruess2005) for a version of TOUGH2 that uses the updated equation of state.

The second part of this intercomparison problem was to compare thermophysical properties of the fluids between codes. Phase densities and viscosities, as well as equilibrium solubilities, were compared for various pressures at a specified temperature of $T = 45^{\circ}$C. The results obtained using PorousFlow are summarized in [tab:properties_water] and [tab:properties_brine] for water and brine, respectively.

!table id=tab:properties_water caption=Phase properties and equilibrium solubilities for water (zero salinity)
| Pressure (MPa)| $\rho_l$ (kg m$^{-3}$) | $\mu_l$ ($\times 10^{-4}$ kg m$^{-1}$ s$^{-1}$)| $\rho_g$ (kg m$^{-3}$) | $\mu_g$ ($\times 10^{-5}$  kg m$^{-1}$ s$^{-1}$) | $X_{CO2}$ | $Y_{H2O}$ |
| - | - | - | - | - | - | - |
| 12 | 1006.35 | 5.978 | 657.87 | 5.127 | 0.052 | 0.0021 |
| 16 | 1008.72 | 5.986 | 759.86 | 6.470 | 0.055 | 0.0024 |
| 20 | 1010.90 | 5.993 | 812.65 | 7.336 | 0.058 | 0.0025 |
| 24 | 1012.73 | 6.001 | 849.38 | 8.030 | 0.059 | 0.0026 |

The phase properties for water and CO$_2$ are nearly identical to the results presented in [!cite](pruess2002)
for the TOUGH2 simulator [!citep](pruess1999), as expected. This is because the mutual solubility model implemented in PorousFlow is the same as that used in TOUGH2, see the [brine-co2](brineco2.md) documentation for details.

!table id=tab:properties_brine caption=Phase properties and equilibrium solubilities for brine ($X_{NaCl} = 0.15$)
| Pressure (MPa)| $\rho_l$ (kg m$^{-3}$) | $\mu_l$ ($\times 10^{-4}$ kg m$^{-1}$ s$^{-1}$)| $\rho_g$ (kg m$^{-3}$) | $\mu_g$ ($\times 10^{-5}$  kg m$^{-1}$ s$^{-1}$) | $X_{CO2}$ | $Y_{H2O}$ |
| - | - | - | - | - | - | - |
| 12 | 1102.96 | 8.286 | 657.87 | 5.127 | 0.025 | 0.0019 |
| 16 | 1104.72 | 8.296 | 759.86 | 6.470 | 0.027 | 0.0022 |
| 20 | 1106.39 | 8.307 | 812.65 | 7.336 | 0.028 | 0.0023 |
| 24 | 1108.02 | 8.317 | 849.38 | 8.030 | 0.029 | 0.0024 |

Similarly, the phase properties for brine and CO$_2$ are very similar to those obtained using TOUGH2 [!citep](pruess2002), again as expected.

!bibtex bibliography
