# Fluid Properties Module

The Fluid Properties module provides a consistent interface to fluid properties such as density,
viscosity, enthalpy and many others, as well as derivatives with respect to the primary
variables. The consistent interface allows different fluids to be used in an input file by simply
swapping the name of the Fluid Properties UserObject in a plug-and-play manner.

## Fluids available

This module provides fluid properties for many different gases, liquids and mixtures.

!syntax list /Modules/FluidProperties groups=FluidPropertiesApp heading-level=3 actions=False

## Properties available

Each FluidProperties UserObject above provides access to several fluid properties. Two main
formulations are available using

- (v, e) - specific volume and specific internal energy
- (p, T) - pressure and temperature

although some fluid properties may be available in terms of other primary variables.
Depending on the actual fluid implementation, any number of the following fluid properties
may be available.

- Density: `rho_from_p_s(pressure, entropy)`
- Density: `rho_from_p_T(pressure, temperature)`
- Dynamic viscosity: `mu_from_v_e(volume, energy)`
- Dynamic viscosity: `mu_from_p_T(pressure, temperature)`
- Enthalpy: `h_from_p_T(pressure, temperature)`
- Enthalpy: `h_from_T_v(temperature, volume)`
- Fluid name: `fluidName()`
- Gibbs free energy: `g_from_v_e(volume, energy)`
- Internal energy: `e_from_v_h(volume, enthalpy)`
- Internal energy: `e_from_p_rho(pressure, density)`
- Internal energy: `e_from_T_v(temperature, volume)`
- Internal energy: `e_from_p_T(pressure, temperature)`
- Isobaric specific heat: `cp_from_v_e(volume, energy)`
- Isobaric specific heat: `cp_from_p_T(pressure, temperature)`
- Isochoric specific heat: `cv_from_v_e(volume, energy)`
- Isochoric specific heat: `cv_from_T_v(temperature, volume)`
- Isochoric specific heat: `cv_from_p_T(pressure, temperature)`
- Molar mass (kg/mol): `molarMass()`
- Pressure: `p_from_v_e(volume, energy)`
- Pressure: `p_from_T_v(temperature, volume)`
- Pressure: `p_from_h_s(pressure, entropy)`
- Ratio of specific heats: `gamma(volume, energy)`
- Ratio of specific heats: `gamma_from_p_T(pressure, temperature)`
- Specific entropy: `s_from_v_e(volume, energy)`
- Specific entropy: `s_from_p_T(pressure, temperature)`
- Specific entropy: `s_from_h_p(enthalpy, pressure)`
- Specific entropy: `s_from_T_v(temperature, volume)`
- Specific volume: `v_from_p_T(pressure, temperature)`
- Speed of sound: `c_from_v_e(volume, energy)`
- Speed of sound: `c_from_p_T(pressure, temperature)`
- Temperature: `T_from_v_e(volume, energy)`
- Temperature: `T_from_p_h(pressure, enthalpy)`
- Thermal conductivity: `k_from_v_e(volume, energy)`
- Thermal conductivity: `k_from_p_T(pressure, temperature)`
- Thermal expansion coefficient: `beta_from_p_T(pressure, temperature)`

Derivatives of fluid properties with respect to the primary variables are also available
for several of the fluid properties listed above. These can be evaluated using the
following notation: `rho_from_p_T(p, T, rho, drho_dp, drho_dT)` etc.

!alert note
Fluid properties are now available using an interface suitable for use with MOOSE's
Automatic Differentiation capability. See example in the next section.

The full list of available methods can be found in either the source code or the
[Modules Doxygen](http://mooseframework.org/docs/doxygen/modules/classes.html) page for each
FluidProperties class.

!alert note
Fluid properties are implemented in GeneralUserObjects that have empty initialize(), execute()
and finalize() methods, so do nothing during a simulation. Their purpose is to provide
convenient access to fluid properties through the existing UserObject interface.

## Usage

All Fluid Properties UserObjects can be accessed in MOOSE objects through the usual UserObject
interface. The following example provides a detailed explanation of the steps involved to use the
Fluid Properties UserObjects in other MOOSE objects, and the syntax required in the input file.

This example is for a problem that has energy-volume as the primary variables. A material is
provided to calculate fluid properties at the quadrature points.

### Source

To access the fluid properties defined in the Fluid Properties module in a MOOSE object, the
source code of the object must include the following lines of code.

In the header file of the material, a `const` reference to the base `SinglePhaseFluidProperties`
object is required:

!listing modules/fluid_properties/include/materials/FluidPropertiesMaterial.h line=SinglePhaseFluidProperties

!alert note
A forward declaration to the `SinglePhaseFluidProperties` class is required at the beginning of
the header file.

!listing modules/fluid_properties/include/materials/FluidPropertiesMaterial.h line=class SinglePhaseFluidProperties

In the source file, the `SinglePhaseFluidProperties` class must be included

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C line= "SinglePhaseFluidProperties.h"

The Fluid Properties UserObject is passed to this material in the input file by adding a
UserObject name parameters in the input parameters:

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C line=addRequiredParam

The reference to the UserObject is then initialized in the constructor using

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C line=getUserObject

The properties defined in the Fluid Properties UserObject can now be accessed through the
reference. In this material, the `computeQpProperties` method calculates a number of properties
at the quadrature points using the values of `_v[_qp]` and `_e[_qp]`.

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterial.C start=computeQpProperties

In a similar fashion, fluid properties can be accessed using the Automatic Differentiation interface
using the `DualReal` version which provides both the value and derivatives

```
DualReal rho = _fp.p_from_T_v(T, v);
```

where $T$ and $v$ are `DualReal`'s. The result (density `rho` in this example) then contains both the
value of density and its derivatives with respect to the primary variables `T` and `v`.

### Input file syntax

The Fluid Properties UserObjects are implemented in an input file in the `Modules` block.  For
example, to use the ideal gas formulation for specific volume and energy, the input file syntax
would be:

!listing modules/fluid_properties/test/tests/ideal_gas/test.i block=Modules

In this example, the user has specified a value for `gamma` (the ratio of isobaric to isochoric
specific heat capacites), and `R`, the universal gas constant.

The fluid properties can then be accessed by other MOOSE objects through the name given in the
input file.

!listing modules/fluid_properties/test/tests/ideal_gas/test.i block=Materials

Due to the consistent interface for fluid properties, a different fluid can be substituted in the
input file be changing the type of the UserObject. For example, to use a stiffened gas instead
of an ideal gas, the only modification required in the input file is

!listing modules/fluid_properties/test/tests/stiffened_gas/test.i block=Modules

## Creating additional fluids

New fluids can be added to the Fluid Properties module by inheriting from the base class and
overriding the methods that describe the fluid properties. These can then be used in an
identical manner as all other Fluid Properties UserObjects.

## Utilities

### Fluid Properties Interrogator

The [FluidPropertiesInterrogator](/FluidPropertiesInterrogator.md) is a user
object which can be used to query eligible fluid properties objects.

## Additional objects

Several additional objects such as AuxKernels and Materials are provided:

!syntax list /AuxKernels groups=FluidPropertiesApp heading-level=3 actions=False

!syntax list /Materials groups=FluidPropertiesApp heading-level=3 actions=False
