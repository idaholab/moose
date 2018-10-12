# Solid Properties Module

The Solid Properties module provides a consistent interface to solid properties. Unlike
fluid properties that are often times only a function of pressure and temperature
(or two other state variables), solid properties are frequently a function of many
other parameters in addition to temperature, such as accumulated radiation damage,
oxidation state, and microstructure. The Solid Properties module provides a consistent
interface to commonly-required solid properties; such properties include thermal
properties such as specific heat, thermal conductivity, and emissivity; and
mechanical properties such as elastic modulus and Poisson ratio. This module uses a
flexible design to permit extension by other applications to include dependencies on
application-specific parameters. In addition, a consistent interface exists to access
derivatives with respect to the primary variables. This module allows different solids
to be used in the input files by simply swapping the name of the Solid Properties
UserObject in a plug-and-play manner.

!alert warning
Always verify that the material properties provided in this module agree with
the system you are modeling. The selection of particular correlations here does not
constitute an endorsement of the accuracy of those correlations.

!alert note
Solid properties are implemented in GeneralUserObjects that have empty initialize(), execute() and
finalize() methods, so do nothing during a simulation. Their purpose is to provide convenient access
to solid properties through the UserObject interface.

This module provides solid properties for different solids, organized according to
the overall type of property.

## Solid properties

All solid properties inherit from the `SolidProperties` base class, which provides
interfaces for providing:

- Molar mass: `molarMass()`
- Solid name: `solidName()`

## Thermal properties

Thermal properties (density, specific heat, thermal conductivity, and emissivity) are
available in terms of temperature. The `ThermalSolidProperties` class inherits
from the `SolidProperties` class, and provides the following additional
properties (with corresponding method names):

- Isobaric specific heat: `cp_from_T(temperature)`
- Thermal conductivity: `k_from_T(temperature)`
- Density: `rho_from_T(temperature)`
- Emissivity: `emissivity_from_T(temperature)`

UserObjects available in the SolidProperties module that provide thermal properties
are:

- [functional](/ThermalFunctionSolidProperties.md)
- [graphite](/ThermalGraphiteProperties.md)
- [silicon carbide](/ThermalSiliconCarbideProperties.md)
- [stainless steel alloy 316](/ThermalStainlessSteel316Properties.md)

!alert note
Additional classes of solid properties, such as mechanical properties, are not yet
implemented.

## Usage

All Solid Properties UserObjects can be accessed in MOOSE objects through the usual UserObject
interface. The following example provides a detailed explanation of the steps involved to use the
Solid Properties UserObjects in other MOOSE objects, and the syntax required in the input file.

This example is for a problem where thermal solid properties are needed. A material
is provided to calculate solid properties at the quadrature points.

### Source

To access the solid properties defined in the Solid Properties module in a MOOSE object, the source
code of the object must include the following lines of code.

In the header file of the material, a `const` reference to the base `ThermalSolidProperties`
object is required:

!listing modules/solid_properties/include/materials/ThermalSolidPropertiesMaterial.h line=ThermalSolidProperties

!alert note
A forward declaration to the `ThermalSolidProperties` class is required at the beginning of the
header file.

!listing modules/solid_properties/include/materials/ThermalSolidPropertiesMaterial.h line=class ThermalSolidProperties

In the include file, the `ThermalSolidProperties` class must be included

!listing modules/solid_properties/include/materials/ThermalSolidPropertiesMaterial.h line= "ThermalSolidProperties.h"

The Solid Properties UserObject is passed to this material in the input file by adding a UserObject
name parameters in the input parameters:

!listing modules/solid_properties/src/materials/ThermalSolidPropertiesMaterial.C line=addRequiredParam

The reference to the UserObject is then initialized in the constructor using

!listing modules/solid_properties/src/materials/ThermalSolidPropertiesMaterial.C line=getUserObject

The properties defined in the Solid Properties UserObject can now be accessed through the
reference. In this material, the `computeQpProperties` method calculates a number of properties at
the quadrature points using the values of `_T[_qp]`.

!listing modules/solid_properties/src/materials/ThermalSolidPropertiesMaterial.C start=computeQpProperties

Applications requiring property derivatives can inherit from the
`ThermalSolidPropertiesMaterial` class and override the
`computeQpProperties()` method to compute such derivatives.

### Input file syntax

The Solid Properties UserObjects are implemented in an input file in the `Modules` block.  For
example, to use stainless steel 316 thermal properties,
the input file syntax would be:

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=Modules

In this example, the user has specified that the surface is polished, rather
than the default `oxidized` state.

The solid properties can then be accessed by other MOOSE objects through the name given in the input
file.

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=Materials

Due to the consistent interface for solid properties, a different solid can be substituted in the
input file be changing the type of the UserObject. For example, to set thermal properties
with an arbitrary functional dependence, the solid property module section of
the input file is:

!listing modules/solid_properties/test/tests/functional/test.i block=Modules

## Creating additional solids

New solids can be added to the Solid Properties module by inheriting from the base class appropriate
to the formulation and overriding the methods that describe the solid properties. These can then be
used in an identical manner as all other Solid Properties UserObjects.

## Objects, Actions, and Syntax

!syntax complete group=SolidPropertiesApp level=3 actions=False
