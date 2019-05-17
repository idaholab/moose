# PorousFlowSingleComponentFluid

!syntax description /Materials/PorousFlowSingleComponentFluid

## Recommended choice of units

Most of MOOSE is independent of the choice of units, but the units must be kept consistent throughout the input file.

- For instance, time can be measured in seconds, nano-seconds, years, etc, providing this choice is used everywhere.

- For instance, you must specify fluid bulk modulus and solid Young's modulus using the same units (for example, both in GPa).

!alert note
PorousFlow does not check the consistency of units in the input file.

However, the [FluidProperties module](fluid_properties/index.md) assumes that the pressure units are Pascals, the time units are seconds, temperature is measured in Kelvin, distance is measured in metres, mass in kilograms and energy in Joules.  Therefore, when using this module, users are ordinarily restricted to using these standard SI units.  Almost all PorousFlow input files use the FluidProperties module.  Therefore it is recommended to use these standard SI units everywhere in your input file.

## Alternate unit choices

`PorousFlowSingleComponentFluid` allows different units to be employed: it modifies the input and output of the FluidProperties module to reflect the choice of units.

- For instance, suppose you choose to measure temperature in Celsius.  Then `PorousFlowSingleComponentFluid` will add 273.15 to your temperature before feeding it to the FluidProperties module.  No other alterations to the FluidProperties input/output are needed.

- For instance, suppose you choose to measure pressure in MPa.  Then `PorousFlowSingleComponentFluid` will multiply your pressure by $10^6$ before feeding it to the FluidProperties module.  Since the viscosity returned by the FluidProperties module has units Pa.s, it needs to be converted to MPa.s before using it in the remainder of PorousFlow, so the FluidProperties viscosity is multiplied by $10^{-6}$.

!alert note
If you choose non-default SI units, you must ensure that all the other parameters in your input file use these non-default units.  For instance, if you choose to measure pressure in MPa, then your solid-mechanical stresses, Young's moduli, strengths, etc, must also be specified in MPa.  That is, `PorousFlowSingleComponentFluid` only interfaces with the FluidProperties module: it doesn't also change your pressure boundary conditions, end time, gravity, etc: all these must be specified by you in the units you choose.

## An essay on Pascals, kilograms and densities

In most PorousFlow simulations, it is convenient to use the unit system: pressure, distance, time, and energy.  Note that mass is not included since
\begin{equation}
\mathrm{kg} = \mathrm{Pa.m.s}^{2} \ .
\end{equation}
Using Pascals rather than kilograms is a different convention to most other physics, and means that, strictly, masses and densities should be thought of in terms of Pascals.

This is quite annoying, however, for if something has a mass of 10$\,$kg, and you want to employ MPa instead of Pa, strictly you should write the mass as 10$^{-5}\,$MPa.m.s$^{2}$.  Similar remarks hold for density (kg.m$^{-3}$), enthalpy and internal energy (J.kg$^{-1}$), heat capacity (J.kg$^{-1}$.K$^{-1}$), and mass fluxes (kg.m$^{-3}$.s$^{-1}$).  Everything gets changed!

Fortuately, density appears as a multiplicative factor almost everywhere, so scaling it by a constant amount (eg, multiplying by 10$^{-6}$ when using MPa instead of Pa) leads to the same equations.  For consider the fluid equations:
\begin{equation}
0 = \frac{\partial}{\partial t}\phi S \rho  + \phi S \rho \nabla\cdot \mathbf{v}_{\mathrm{s}} + \nabla \cdot \rho \mathbf{v}_{\mathrm{Darcy}} + \nabla\cdot \rho D \nabla \chi - q \ .
\end{equation}
Obviously, scaling $\rho$ will lead to the same equation.  The only thing to be careful of is the $\rho g$ inside the Darcy velocity, which is discussed below.  The heat equation is
\begin{equation}
0 = \frac{\partial}{\partial t}\phi S \rho \mathcal{E}  + \phi S \rho\mathcal{E} \nabla\cdot \mathbf{v}_{\mathrm{s}} + \nabla \cdot \rho h \mathbf{v}_{\mathrm{Darcy}} - \nabla\cdot \lambda \nabla T - q^{\mathrm{T}} \ .
\end{equation}
Since $\rho$ scales inversely to $\mathcal{E}$ (with respect to mass and pressure) the quotient $\rho\mathcal{E}$ is independent of the mass unit (it has units energy.distance$^{-3}$).  The same remark holds for $\rho h$.  The solid-mechanics conservation of momentum reads
\begin{equation}
\rho_{\mathrm{mat}}\frac{\partial v_{\mathrm{s}}}{\partial t} = \nabla \sigma^{\mathrm{tot}} + \rho_{\mathrm{mat}} b \ .
\end{equation}
Here, the $\rho_{\mathrm{mat}}$ on the left-hand side must be measured in the "Pascals" units described above.  The $\rho_{\mathrm{mat}}$ on the right-hand side should be too, although often the body force is due to gravity, so may be treated as described below.

### Suggested approach

Continue to think in terms of kilograms:

- specify fluid density in kg.m$^{-3}$

- specify enthalpy and internal energy in J.kg$^{-1}$

- specify mass fluxes in kg.m$^{-3}$.s$^{-1}$

- specify heat capacity in J.kg$^{-1}$.K$^{-1}$

but modify gravity and the acceleration terms in the solid-mechanics equations, as described below.

!alert warning
This is the approach used by `PorousFlowSingleComponentFluid`.  That is, when you use non-default units, `PorousFlowSingleComponentFluid` does not modify the fluid density, fluid enthalpy and fluid internal energy returned by the FluidProperties module.

### Addressing the issue of gravity in the Darcy velocity and solid-mechanics body force

The product $\rho g$ appears in the Darcy velocity.  The product $\rho_{\mathrm{mat}} b$ appears in the solid-mechanics equation.  Strictly, we should measure $\rho$ in units pressure.time$^{2}$.distance$^{-2}$ (eg, Pa.s$^{2}$.m$^{-2}$) in each of these, but we have found that to be inconvenient, so we modify the accelerations $g$ and $b$ instead, in order to achieve the correct result for the products $\rho g$ and $\rho_{\mathrm{mat}} b$.  [tab:grav] should help.

!table id=tab:grav caption=Examples of $g$ and $b$ to use in the terms $\rho g$ and $\rho_{\mathrm{mat}}b$, in different units
| Value | Units used |
| - | - |
| $g = 9.81$ | m.s$^{-2}$ |
| $9.81$ | Pascals (Pa) |
| $9.81\times 10^{-6}$ | MPa |
| $9.81\times 10^{-9}$ | GPa |


### Addressing the issue of the solid-mechanics acceleration

If you use time-dependent solid mechanics, with the term
\begin{equation}
\rho_{\mathrm{mat}}\frac{\partial v_{\mathrm{s}}}{\partial t} \ ,
\end{equation}
you must measure $\rho_{\mathrm{mat}}$ in units of pressure.time$^{2}$.distance$^{-2}$.  [tab:rhomat] should help.

!table id=tab:rhomat caption=Examples of $\rho_{\mathrm{mat}}$ to use in $\rho_{\mathrm{mat}}\frac{\partial v_{\mathrm{s}}}{\partial t}$, in different units
| Value | Units used |
| - | - |
| $\rho_{\mathrm{mat}} = 1234$ | kg.m$^{-3}$ |
| $1234$ | Pascals (Pa), seconds |
| $1234\times 10^{-6}$ | MPa, seconds |
| $9.522\times 10^{-5}$ | Pa, hours |
| $9.522\times 10^{-11}$ | MPa, hours |
| $1.653\times 10^{-7}$ | Pa, days |
| $1.653\times 10^{-13}$ | MPa, days |
| $1.241\times 10^{-12}$ | MPa, years |
| $1.241\times 10^{-18}$ | MPa, years |


## Available unit systems

`PorousFlowSingleComponentFluid` includes the following choices for unit systems

### Default: Pascals, seconds and Kelvin

No changes to the input or output of the FluidProperties module are required

### Temperature in Celsius

Choosing the temperature unit to be Celcius means that 273.15 is added to all PorousFlow temperatures before feeding them to the FluidProperties module.

Other input-file objects that may need to be specified in Celsius are boundary conditions, initial conditions, [PorousFlowSinks](PorousFlowSink.md), etc.

All objects in the input file need to be specified in Celsius, except the FluidProperties objects.  For example, if using the [SimpleFluidProperties](SimpleFluidProperties.md), then you must remember the $T$ seen by SimpleFluidProperties will be measured in Kelvin.

### Pressure (and stress) in MPa

Choosing the pressure unit to be MPa means that the PorousFlow pressures are multiplied by $10^{6}$ before feeding them to the FluidProperties module.  The viscosity returned by the FluidProperties module is multiplied by $10^{-6}$ (and any quantity from choosing non-default time units) before handing back to the remainder of PorousFlow.

In addition, you must remember that solid mechanical stresses should be measured in MPa, so moduli and strengths need to be specified in MPa.  Similarly, boundary tractions or pressures need to be specified in MPa.

Remember that the FluidProperties module always uses Pa, m, s and J.  Therefore, when using [SimpleFluidProperties](SimpleFluidProperties.md) you must specify the bulk modulus and viscosity in these units, not using MPa.

### Alternate time units

Choosing the time unit to be hours means no change is made to the inputs to the FluidProperties module.

You must remember that fluid and heat sources must be measured in kg.m$^{-3}$.time$^{-1}$ and J.m$^{-3}$.time$^{-1}$, respectively (where "time" is "hours", "days" or "years" depending on your specific choice).  Remember too that the thermal conductivity of the solid skeleton must be measured in J.m$^{-1}$.K$^{-1}$.time$^{-1}$.  Velocities, radioactive decay rates, chemical precipitation and reaction rates, dispersion tensors and diffusion coefficients are similarly impacted.

Remember that the FluidProperties module always uses Pa, m, s and J.  Therefore, when using [SimpleFluidProperties](SimpleFluidProperties.md) you must specify the fluid thermal conductivity and viscosity in these units, not using hours, days or years.

#### Hours

The viscosity returned by the FluidProperties module is multiplied by $1/3600$ (and any quantity from choosing non-default pressure units) before handing back to the remainder of PorousFlow.

#### Days

The viscosity returned by the FluidProperties module is multiplied by $1/(3600\times 24)$ (and any quantity from choosing non-default pressure units) before handing back to the remainder of PorousFlow.

#### Julian years

The viscosity returned by the FluidProperties module is multiplied by $1/(3600\times 24\times 365.25)$ (and any quantity from choosing non-default pressure units) before handing back to the remainder of PorousFlow.


!syntax parameters /Materials/PorousFlowSingleComponentFluid

!syntax inputs /Materials/PorousFlowSingleComponentFluid

!syntax children /Materials/PorousFlowSingleComponentFluid
