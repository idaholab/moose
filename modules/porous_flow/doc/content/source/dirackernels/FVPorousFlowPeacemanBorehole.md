# FVPorousFlowPeacemanBorehole

`FVPorousFlowPeacemanBorehole` is a Peaceman borehole for finite-volume PorousFlow simulations (those using [FVPorousFlowAdvectiveFlux](FVPorousFlowAdvectiveFlux.md) and the AD PorousFlow materials).  As [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md), it approximates a polyline borehole by a sequence of Dirac points, and it accepts the same geometry and multiplier parameters.  The borehole may be controlled by its bottomhole pressure or by its total mass rate.  Please see [sinks](sinks.md) for an extended discussion of the Peaceman borehole formulation.

!alert warning title=Fluid flow only
Heat transfer is not implemented: there are no enthalpy or internal-energy multipliers, and the borehole cannot be a function of temperature.  Use the finite-element [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) for those.

## Formulation

At borehole point $i$, which lies in a cell with porepressure $P_{\beta}$ of phase $\beta$ (the [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/fluid_phase)), the flux out of the porous medium is
\begin{equation}
Q_{i} = |c| \, W_{i} \, M \left(P_{\beta} - P_{\mathrm{bot}} - \gamma \cdot (x_{i} - x^{\mathrm{bot}})\right) \ ,
\end{equation}
where

- $c$ is the [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/character) function;
- $W_{i}$ is Peaceman's well constant of the half segments either side of point $i$, computed as in [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) from the permeability, the cell size, the borehole radius and [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/re_constant), or given by [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/well_constant).  The default `re_constant = 0.28` is Peaceman's value for cell-centred grids, which is the appropriate one for finite volumes;
- $M$ is the product of the multipliers selected by [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/use_mobility) ($\rho_{\beta} k_{\mathrm{r},\beta} / \mu_{\beta}$), [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/use_relative_permeability) ($k_{\mathrm{r},\beta}$), [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/mass_fraction_component) ($\chi_{\beta}^{\kappa}$) and [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/multiplying_var);
- $\gamma$ is the [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/unit_weight), so that $P_{\mathrm{bot}} + \gamma \cdot (x_{i} - x^{\mathrm{bot}})$ is the borehole pressure at point $i$, and $x^{\mathrm{bot}}$ is the last point of the borehole.

The material properties are those of the cell containing the point, and the residual and Jacobian contributions are computed by automatic differentiation.  As for the finite-element boreholes, multiphase and multicomponent boreholes use one `FVPorousFlowPeacemanBorehole` per fluid phase and component, each acting on the variable of that component's mass balance.

### Bottomhole-pressure control

With `control = bhp` (the default), $P_{\mathrm{bot}}$ is the function [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/bottom_p_or_t), evaluated at the bottom of the borehole.

### Rate control

With `control = rate`, the total mass rate $Q_{\mathrm{target}}(t)$ of the borehole is the function [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/rate) (positive for production, negative for injection), and $P_{\mathrm{bot}}$ is a first-order scalar variable given by [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/bottom_p).  All the `FVPorousFlowPeacemanBorehole` DiracKernels that share `bottom_p` are one borehole, and the equation of `bottom_p` is
\begin{equation}
Q_{\mathrm{target}}(t) - \sum_{\mathrm{DiracKernels}} \sum_{i} Q_{i} = 0 \ ,
\end{equation}
which they assemble together: each adds its fluxes, and one of them adds $Q_{\mathrm{target}}$.  `use_mobility = true` is required, so that the fluxes are mass fluxes.  `bottom_p` is solved fully coupled with the PorousFlow variables, so Newton's method converges quadratically.  The `bottom_p` variable needs a `NullScalarKernel` (to declare that it has an equation), and its initial condition is the starting guess for the bottomhole pressure.

### Flow direction and injection

By default the borehole is a producer or an injector, as [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md): if [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/character) is positive fluid only flows out of the porous medium, and if it is negative fluid only flows in.  Two options change this:

- [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/allow_crossflow) `= true` allows fluid to flow in either direction at every borehole point (the sign of `character` is then ignored);
- with [!param](/DiracKernels/FVPorousFlowPeacemanBorehole/injection_phase) given, the flow direction at each point is decided by the total volumetric flow of all phases, $\sum_{\beta} (k_{\mathrm{r},\beta}/\mu_{\beta}) (P_{\beta} - P_{\mathrm{bot}} - \gamma \cdot (x_{i} - x^{\mathrm{bot}}))$, and at injecting points only the injection phase enters the porous medium, with the total mobility $\rho_{\mathrm{inj}} \sum_{\beta} k_{\mathrm{r},\beta}/\mu_{\beta}$ of the cell.  This allows, for example, gas to be injected into a cell that contains no gas, where the gas relative permeability is zero.

## Example

Gas is injected into a water-saturated reservoir at a prescribed rate, using one DiracKernel for the water (phase 0, component 0) and one for the gas (phase 1, component 1):

!listing modules/porous_flow/test/tests/dirackernels/fv_borehole/fv_borehole_2ph_rate.i block=Variables DiracKernels ScalarKernels

## Comparison with MRST

The borehole has been compared with MRST 2023b [!citep](lie2019mrst) (the `ad-blackoil` module).  The model is a $20 \times 20 \times 30$ m box of $5$ m cubic cells with permeability $10^{-12}$ m$^{2}$, porosity $0.1$, gravity $10$ m s$^{-2}$ and a hydrostatic initial porepressure.  A vertical borehole of radius $0.1$ m has three points at cell centres, $5$ m apart, with half-segment lengths $2.5$, $5$ and $2.5$ m.  Both codes use the two-point flux approximation, the same Peaceman well constant and a wellbore pressure gradient of $10^{4}$ Pa m$^{-1}$, and the simulations use $20$ time steps of $5$ s.  The MOOSE boreholes use `allow_crossflow = true` and, in the two-phase cases, `injection_phase = 1` (the gas), which correspond to the treatment of wells in MRST: in MRST the bottomhole pressure of a rate-controlled well is also solved fully coupled with the reservoir equations.  Water has density $1000 \exp(P/2\times 10^{9})$ kg m$^{-3}$ and viscosity $10^{-3}$ Pa s.  The two-phase cases add an immiscible CO$_{2}$-like gas with density $364 \exp(P/2\times 10^{7})$ kg m$^{-3}$ (about $600$ kg m$^{-3}$ at $10$ MPa) and viscosity $5\times 10^{-5}$ Pa s, Corey relative permeabilities with exponent $2$ and no capillary pressure.

!media media/porous_flow/fv_borehole_1phase.png style=width:90%;margin-left:10px caption=Single phase.  Left: with rate control, the borehole produces 1 kg/s and then injects 0.5 kg/s, and the bottomhole pressure is computed.  Right: with bottomhole-pressure control, the borehole produces and then injects.  id=fv_borehole_1phase.fig

!media media/porous_flow/fv_borehole_2phase_rate.png style=width:90%;margin-left:10px caption=Water and gas, rate control.  Left: gas injection into a reservoir full of water.  Right: production of 1 kg/s from a reservoir with a uniform gas saturation of 0.3, which produces both phases.  id=fv_borehole_2phase_rate.fig

!media media/porous_flow/fv_borehole_2phase_bhp.png style=width:90%;margin-left:10px caption=Water and gas, bottomhole-pressure control.  Left: gas injection rate into a reservoir full of water.  Right: water and gas production rates from a reservoir with a uniform gas saturation of 0.3.  id=fv_borehole_2phase_bhp.fig

The largest differences over the simulations are listed in [fv_borehole_mrst].  The remaining differences in the two-phase production cases are attributed to the gravity term of the inter-cell fluxes, which uses the density of one cell in MOOSE and the average of the two cells in MRST.

!table id=fv_borehole_mrst caption=Largest difference between MOOSE and MRST over the 20 time steps.  With rate control the rate is prescribed in both codes, and with bottomhole-pressure control the bottomhole pressure is prescribed.
| Case | Control | Bottomhole pressure (Pa) | Mass rate (kg/s) |
| --- | --- | --- | --- |
| Single phase | rate | 0.37 | - |
| Single phase | bhp | - | $4 \times 10^{-6}$ |
| Gas injection | rate | 0.21 | - |
| Gas injection | bhp | - | $2 \times 10^{-6}$ |
| Water and gas production | rate | 11 | - |
| Water and gas production | bhp | - | $3 \times 10^{-4}$ |

!syntax parameters /DiracKernels/FVPorousFlowPeacemanBorehole

!syntax inputs /DiracKernels/FVPorousFlowPeacemanBorehole

!syntax children /DiracKernels/FVPorousFlowPeacemanBorehole
