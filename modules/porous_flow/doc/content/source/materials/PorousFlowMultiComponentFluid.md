# PorousFlowMultiComponentFluid

!syntax description /Materials/PorousFlowMultiComponentFluid

## Recommended choice of units

Most of MOOSE is independent of the choice of units, but the units must be kept consistent throughout the input file.

- For instance, time can be measured in seconds, nano-seconds, years, etc, providing this choice is used everywhere.

- For instance, you must specify fluid bulk modulus and solid Young's modulus using the same units (for example, both in GPa).

!alert note
PorousFlow does not check the consistency of units in the input file.

However, the [FluidProperties module](fluid_properties/index.md) assumes that the pressure units are Pascals, the time units are seconds, temperature is measured in Kelvin, distance is measured in metres, mass in kilograms and energy in Joules.  Therefore, when using this module, users are ordinarily restricted to using these standard SI units.  Almost all PorousFlow input files use the FluidProperties module.  Therefore it is recommended to use these standard SI units everywhere in your input file.

## Alternate unit choices

`PorousFlowMultiComponentFluid` allows different units to be employed: it modifies the input and output of the FluidProperties module to reflect the choice of units.

- For instance, suppose you choose to measure temperature in Celsius.  Then `PorousFlowMultiComponentFluid` will add 273.15 to your temperature before feeding it to the FluidProperties module.  No other alterations to the FluidProperties input/output are needed.

- For instance, suppose you choose to measure pressure in MPa.  Then `PorousFlowMultiComponentFluid` will multiply your pressure by $10^6$ before feeding it to the FluidProperties module.  Since the viscosity returned by the FluidProperties module has units Pa.s, it needs to be converted to MPa.s before using it in the remainder of PorousFlow, so the FluidProperties viscosity is multiplied by $10^{-6}$.

!alert note
If you choose non-default SI units, you must ensure that all the other parameters in your input file use these non-default units.  For instance, if you choose to measure pressure in MPa, then your solid-mechanical stresses, Young's moduli, strengths, etc, must also be specified in MPa.  That is, `PorousFlowMultiComponentFluid` only interfaces with the FluidProperties module: it doesn't also change your pressure boundary conditions, end time, gravity, etc: all these must be specified by you in the units you choose. See [PorousFlowSingleComponentFluid](PorousFlowSingleComponentFluid.md) for more details.



## Available unit systems

`PorousFlowMultiComponentFluid` includes the following choices for unit systems

### Default: Pascals, seconds and Kelvin

No changes to the input or output of the FluidProperties module are required

### Temperature in Celsius

Choosing the temperature unit to be Celcius means that 273.15 is added to all PorousFlow temperatures before feeding them to the FluidProperties module.

Other input-file objects that may need to be specified in Celsius are boundary conditions, initial conditions, [PorousFlowSinks](PorousFlowSink.md), etc.

All objects in the input file need to be specified in Celsius, except the FluidProperties objects.

### Pressure (and stress) in MPa

Choosing the pressure unit to be MPa means that the PorousFlow pressures are multiplied by $10^{6}$ before feeding them to the FluidProperties module.  The viscosity returned by the FluidProperties module is multiplied by $10^{-6}$ (and any quantity from choosing non-default time units) before handing back to the remainder of PorousFlow.

In addition, you must remember that solid mechanical stresses should be measured in MPa, so moduli and strengths need to be specified in MPa.  Similarly, boundary tractions or pressures need to be specified in MPa.

Remember that the FluidProperties module always uses Pa, m, s and J.

### Alternate time units

Choosing the time unit to be hours means no change is made to the inputs to the FluidProperties module.

You must remember that fluid and heat sources must be measured in kg.m$^{-3}$.time$^{-1}$ and J.m$^{-3}$.time$^{-1}$, respectively (where "time" is "hours", "days" or "years" depending on your specific choice).  Remember too that the thermal conductivity of the solid skeleton must be measured in J.m$^{-1}$.K$^{-1}$.time$^{-1}$.  Velocities, radioactive decay rates, chemical precipitation and reaction rates, dispersion tensors and diffusion coefficients are similarly impacted.

Remember that the FluidProperties module always uses Pa, m, s and J. 

#### Hours

The viscosity returned by the FluidProperties module is multiplied by $1/3600$ (and any quantity from choosing non-default pressure units) before handing back to the remainder of PorousFlow.

#### Days

The viscosity returned by the FluidProperties module is multiplied by $1/(3600\times 24)$ (and any quantity from choosing non-default pressure units) before handing back to the remainder of PorousFlow.

#### Julian years

The viscosity returned by the FluidProperties module is multiplied by $1/(3600\times 24\times 365.25)$ (and any quantity from choosing non-default pressure units) before handing back to the remainder of PorousFlow.


!syntax parameters /Materials/PorousFlowMultiComponentFluid

!syntax inputs /Materials/PorousFlowMultiComponentFluid

!syntax children /Materials/PorousFlowMultiComponentFluid
