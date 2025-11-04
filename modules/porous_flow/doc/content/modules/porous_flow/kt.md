# How to use Kuzmin-Turek stabilization in PorousFlow simulations

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)

Kuzmin and Turek [!citep](KuzminTurek2004) describe a method of stabilising advection while minimising artificial numerical diffusion.  In this page "Kuzmin and Turek" is abbreviated to "KT".   Basic features of KT's approach are detailed in [a worked example](kt_worked.md) and the results are compared with full upwinding, RDG and no stabilization in the [numerical diffusion page](numerical_diffusion.md).

This page is for users who want to use KT stabilization in PorousFlow simulations.

!alert note
The Kuzmin-Turek stabilization is new and users are strongly encouraged to experiment with it and report their findings to the moose-users gmail group to iron out any problems and so we can collectively gain experience.

## Using the Action system

Both the [PorousFlowFullySaturated](actions/PorousFlowFullySaturated.md) and [PorousFlowUnsaturated](actions/PorousFlowUnsaturated.md) Actions support KT stabilization (the [PorousFlowBasicTHM](actions/PorousFlowBasicTHM.md) Action does not support any numerical stabilization as it is typically unneeded).  KT stabilization may be included simply by specifying the `stabilization` and `flux_limiter_type` parameters.  For instance:

!listing modules/porous_flow/examples/tutorial/06_KT.i block=PorousFlowFullySaturated

and

!listing modules/porous_flow/examples/tutorial/08_KT.i block=PorousFlowUnsaturated

## Using Kernels and UserObjects

It is also fairly straightforward to swap from full-upwinding (or no stabilization) to KT stabilization.  Only two changes are needed in the input file.

### Kernels

Instead of [PorousFlowAdvectiveFlux](PorousFlowAdvectiveFlux.md) or [PorousFlowHeatAdvection](PorousFlowHeatAdvection.md) or another type of PorousFlow Kernel, the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel must be used.  For instance, a full-upwind simulation's Kernels are:

!listing modules/porous_flow/test/tests/heat_advection/heat_advection_1d.i block=Kernels

while an identical simulation using KT stabilization has Kernels:

!listing modules/porous_flow/test/tests/heat_advection/heat_advection_1d_KT.i block=Kernels

Notice that the `PorousFlowFluxLimitedTVDAdvection` Kernels require an `advective_flux_calculator` input.  This is a UserObject, which is the subject of the next section.

### UserObjects

The set of PorousFlow advective flux calculator UserObjects compute the Residual and Jacobian entries that are used by the [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernels.  The reason that these entries are computed by UserObjects instead of Kernels is that the KT scheme requires information about porepressures, etc, that are 2 nodes away from the current node.  This information gets assembled and processed to determine upwinding directions and suitable stabilization approaches before the residual and Jacobian are computed.  The process is explained in the [worked example of KT stabilization](kt_worked.md) page.  The upshot is that a UserObject must be employed instead of the usual MOOSE Kernel-only approach.

!table id=uotypes caption=Available types of PorousFlow advective flux UserObjects
| Equation | UserObject | Use case |
| --- | --- | --- |
| $-\frac{\rho}{\mu}k_{ij}(\nabla_{j}P - \rho g_{j})$ | [PorousFlowAdvectiveFluxCalculatorSaturated](PorousFlowAdvectiveFluxCalculatorSaturated.md) | 1-phase, 1-component, fully-saturated |
| $-\chi^{\kappa}\frac{\rho}{\mu}k_{ij}(\nabla_{j}P - \rho g_{j})$ | [PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent](PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent.md) | 1-phase, multi-component, fully-saturated |
| $-k_{\mathrm{r}}\frac{\rho}{\mu}k_{ij}(\nabla_{j}P - \rho g_{j})$ | [PorousFlowAdvectiveFluxCalculatorUnsaturated](PorousFlowAdvectiveFluxCalculatorUnsaturated.md) | 1-phase, 1-component, unsaturated |
| $-k_{\mathrm{r,}\beta}\frac{\rho_{\beta}}{\mu_{\beta}}k_{ij}(\nabla_{j}P_{\beta} - \rho_{\beta} g_{j})$ | [PorousFlowAdvectiveFluxCalculatorUnsaturated](PorousFlowAdvectiveFluxCalculatorUnsaturated.md) | multi-phase, multi-component, immiscible |
| $-k_{\mathrm{r,}\beta}\chi_{\beta}^{\kappa}\frac{\rho_{\beta}}{\mu_{\beta}}k_{ij}(\nabla_{j}P_{\beta} - \rho_{\beta} g_{j})$ | [PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent](PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent.md) | multi-phase, multi-component |
| $-h\frac{\rho}{\mu}k_{ij}(\nabla_{j}P - \rho g_{j})$ | [PorousFlowAdvectiveFluxCalculatorSaturatedHeat](PorousFlowAdvectiveFluxCalculatorSaturatedHeat.md) | heat, 1-phase |
| $-h_{\beta}k_{\mathrm{r,}\beta}\frac{\rho_{\beta}}{\mu_{\beta}}k_{ij}(\nabla_{j}P_{\beta} - \rho_{\beta} g_{j})$ | [PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat](PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat.md) | heat, multi-phase |

The notation is detailed in the [nomenclature](porous_flow/nomenclature.md): briefly, a $\beta$ subscript denotes a phase, a $\kappa$ superscript denotes a fluid component, $\rho$ is a fluid density, $\mu$ a fluid viscosity, $k_{ij}$ the permeability tensor, $P$ a fluid porepressure, $g_{j}$ the acceleration of gravity, $\chi$ a mass fraction, $k_{\mathrm{r}}$ a relative permeability, and $h$ an enthalpy.

Which UserObjects you need depends on your simulation.  For example, in the case of a single-phase, single-component, fully-saturated anisothermal simulation we need two UserObjects: one for the advection of the fluid, and one for the advection of the heat.

!listing modules/porous_flow/test/tests/heat_advection/heat_advection_1d_KT.i start=[fluid_advective_flux] end=[Modules]

Clicking on the links in the above table will provide you with more examples.  For instance the following set of Kernels and UserObjects illustrates the case of a single tracer in a fully-saturated single-phase fluid:

!listing modules/porous_flow/test/tests/flux_limited_TVD_pflow/pffltvd_1D.i block=Kernels

!listing modules/porous_flow/test/tests/flux_limited_TVD_pflow/pffltvd_1D.i start=[advective_flux_calculator_0] end=[Materials]

!alert note
For multi-phase situations you will need an advective flux calculator for each phase and each component, unless the phases are immiscible (each component exists in one phase only).

For example, in the case of 2 phases with 2 components, each potentially existing in both phases, there are 2 PorousFlow [governing equations](porous_flow/governing_equations.md): one for each component.  The equation for fluid component zero contains contributions from both phase 0 and phase 1.  The equation for fluid component one contains contributions from both phase 0 and phase 1.  So the Kernels will look like

!listing modules/porous_flow/test/tests/pressure_pulse/pressure_pulse_1d_2phasePS_KT.i start=Kernels

and the UserObjects will look like

!listing modules/porous_flow/test/tests/pressure_pulse/pressure_pulse_1d_2phasePS_KT.i start=[afc_component0_phase0] end=[Modules]
