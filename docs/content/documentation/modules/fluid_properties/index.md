# Fluid Properties Module
The Fluid Properties module provides a consistent interface to fluid properties such as
density, viscosity, enthalpy and many others, as well as derivatives with respect to the
primary variables. This allows different fluids to be used in the input files by simply
swapping the name of the Fluid Properties UserObject in a plug-and-play manner.

!!! note
    Fluid properties are implemented in GeneralUserObjects that have empty initialize(),
    execute() and finalize() methods, so do nothing during a simulation. Their purpose is
    to provide convenient access to fluid properties through the UserObject interface.

This module provides fluid properties for different liquids and gases.

Provided formulations are available in terms of:

* (p, T) - pressure and temperature
* (v, e) - specific volume and specific internal energy

##Energy-volume formulation
For the energy-specific volume formulation, the following properties (with
corresponding method names) are provided:

* Pressure: `pressure(volume, energy)`
* Temperature: `temperature(volume, energy)`
* Speed of sound: `c(volume, energy)`
* Isobaric specific heat: `cp(volume, energy)`
* Isochoric specific heat: `cp(volume, energy)`
* Ratio of specific heats: `gamma(volume, energy)`
* Dynamic viscosity: `mu(volume, energy)`
* Thermal conductivity: `k(volume, energy)`
* Specific entropy: `s(volume, energy)`
* Density: `rho(pressure, temperature)`
* Enthalpy: `h(pressure, temperature)`
* Internal energy: `e(pressure, density)`
* Thermal expansion coefficient: `beta(pressure, temperature)`
* Pressure (from enthalpy and entropy): `p_from_hs(enthalpy, entropy)`

UserObjects available in the FluidProperties module that use the internal energy-volume
formulation are

* [Ideal gas](/IdealGasFluidProperties.md)
* [Stiffened gas](/StiffenedGasFluidProperties.md)

This formulation is useful for the [Navier-Stokes](modules/navier_stokes/index.md) module.

##Pressure-temperature formulation
A separate formulation based on pressure (Pa) and temperature (K) is also provided.
For the pressure-temperature formulation, the available properties (with corresponding
method names) are:

* String representing fluid name: `fluidName()`
* Molar mass (kg/mol): `molarMass()`
* Density (kg/m$^3$): `rho(pressure, temperature)`
* Internal energy (J/kg): `e(pressure, temperature)`
* Enthalpy (J/kg): `h(pressure, temperature)`
* Specific entropy (J/kg/K): `s(pressure, temperature)`
* Dynamic viscosity (Pa.s): `mu(pressure, temperature)` or `mu(density, temperature)`
* Thermal conductivity (W/m/K): `k(pressure, temperature)` or `k(density, temperature)`
* Isobaric specific heat (J/kg/K): `cp(pressure, temperature)`
* Isochoric specific heat (J/kg/K): `cv(pressure, temperature)`
* Ratio of heat capacites (-): `gamma(pressure, temperature)`
* Thermal expansion coefficient (-): `beta(pressure, temperature)`
* Henry's law constant (1/Pa): `henry(temperature)`

Available fluids are:

* [Ideal gas](/IdealGasFluidPropertiesPT.md)
* [Simple fluid](/SimpleFluidProperties.md)
* [Water](/Water97FluidProperties.md)
* [Methane](/MethaneFluidProperties.md)
* [Carbon dioxide](/CO2FluidProperties.md)
* [NaCl](/NaClFluidProperties.md)
* [Brine (water and salt)](/BrineFluidProperties.md)
* [Tabulated](/TabulatedFluidProperties.md)
* [Sodium](/SodiumProperties.md)

These fluid properties can be used directly in the [Porous Flow](modules/porous_flow/index.md) module.

## Usage
All Fluid Properties UserObjects can be accessed in MOOSE objects through the usual
UserObject interface. The following example provides a detailed explanation of the steps
involved to use the Fluid Properties UserObjects in other MOOSE objects, and the syntax
required in the input file.

This example is for a problem that has energy-volume as the primary variables. A material is
provided to calculate fluid properties at the quadrature points.

For problems that use the pressure-temperature formulation, the procedure for using
the Fluid Properties UserObjects is identical, apart from a change in the base class name
(from `SinglePhaseFluidProperties` to `SinglePhaseFluidPropertiesPT`).

###Source
To access the fluid properties defined in the Fluid Properties module in a MOOSE object,
the source code of the object must include the following lines of code.

In the header file of the material, a `const` reference to
the base `SinglePhaseFluidProperties` object is required:

!listing modules/fluid_properties/include/materials/FluidPropertiesMaterial.h line=SinglePhaseFluidProperties

Note: a forward declaration to the `SinglePhaseFluidProperties` class is required at
the beginning of the header file.

!listing modules/fluid_properties/include/materials/FluidPropertiesMaterial.h line=class SinglePhaseFluidProperties

In the source file, the `SinglePhaseFluidProperties` class must be included

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C line= "SinglePhaseFluidProperties.h"

The Fluid Properties UserObject is passed to this material in the input file by adding
a UserObject name parameters in the input parameters:

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C line=addRequiredParam

The reference to the UserObject is then initialized in the constructor using

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C line=getUserObject

The properties defined in the Fluid Properties UserObject can now be accessed through
the reference. In this material, the `computeQpProperties` method calculates a number of
properties at the quadrature points using the values of `_v[_qp]` and `_e[_qp]`.

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C start=computeQpProperties

###Input file syntax
The Fluid Properties UserObjects are implemented in an input file in the `Modules` block.
For example, to use the ideal gas formulation for specific volume and energy, the input
file syntax would be:

!listing modules/fluid_properties/test/tests/ideal_gas/test.i block=Modules label=False

In this example, the user has specified a value for `gamma` (the ratio of isobaric
to isochoric specific heat capacites), and `R`, the universal gas constant.

The fluid properties can then be accessed by other MOOSE objects through the name
given in the input file.

!listing modules/fluid_properties/test/tests/ideal_gas/test.i block=Materials label=False

Due to the consistent interface for fluid properties, a different fluid can be substituted
in the input file be changing the type of the UserObject. For example, to use a stiffened
gas instead of an ideal gas, the only modification required in the input file is

!listing modules/fluid_properties/test/tests/stiffened_gas/test.i block=Modules label=False

## Creating additional fluids
New fluids can be added to the Fluid Properties module by inheriting from the
base class appropriate to the formulation and overriding the methods that describe
the fluid properties. These can then be used in an identical manner as all other
Fluid Properties UserObjects.
