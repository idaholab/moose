# Solid Properties Module

The Solid Properties module provides a consistent interface to solid properties. Unlike
fluid properties that are oftentimes only a function of pressure and temperature
(or two other state variables), solid properties are frequently a function of many
other parameters in addition to temperature, such as accumulated radiation damage,
oxidation state, and microstructure. Some common properties include thermal
properties such as specific heat, thermal conductivity, and emissivity; and
mechanical properties such as elastic modulus and Poisson ratio. This module uses a
flexible design to permit extension by other applications to include dependencies on
application-specific parameters, but with some common use cases to provide derivable interfaces.
This module allows different solids
to be used in the input files by simply swapping the name of the solid properties
userobject in a plug-and-play manner.

!alert warning
Always verify that the material properties provided in this module agree with
the system you are modeling. The selection of particular correlations here does not
constitute an endorsement of the accuracy of those correlations.

This module provides solid properties for different solid materials, organized according to
the overall type of property.

## Thermal properties

Thermal properties (density, specific heat, and thermal conductivity) are computed by
userobjects inheriting from the [ThermalSolidProperties](/userobjects/ThermalSolidProperties.md) base class. This class
defines functions to compute these three material properties as a function of temperature.

Then, in the [ThermalSolidProperties](/userobjects/ThermalSolidProperties.md) user object, the `computeQpProperties` method calculates a number of properties at
the quadrature points using the values of `_temperature[_qp]` and the functions provided by
the selected userobject.
For flexibility, a custom name may be provided for each of these three properties.

!listing modules/solid_properties/src/materials/ThermalSolidPropertiesMaterial.C start=computeQpProperties

The calculation of material properties is divided into several different methods:

- compute isobaric specific heat - `cp_from_T(const Real & T)`
- compute thermal conductivity - `k_from_T(const Real & T)`
- compute density - `rho_from_T(const Real & T)`

An example is provided below for creating a new solid material providing thermal properties.

Materials available in the Solid Properties module that provide thermal properties
are:

- [graphite](/ThermalGraphiteProperties.md)
- [monolithic silicon carbide](/ThermalMonolithicSiCProperties.md)
- [composite silicon carbide](/ThermalCompositeSiCProperties.md)
- [stainless steel alloy 316](/ThermalSS316Properties.md)
- [using functions](/ThermalFunctionSolidProperties.md)

!alert note
Additional classes of solid properties, such as mechanical properties, are not yet
implemented.

## Usage

All solid properties materials can be accessed in MOOSE objects through the usual Material
interface. The following example provides a detailed explanation of the steps involved to use the
solid properties materials in other MOOSE objects, and the syntax required in the input file.

This example is for a problem where thermal conductivity is needed in a kernel.
An example input file syntax using this kernel and the Solid Properties module is also provided.
Similar steps would be followed for other material properties.

### Source

To access the solid properties defined in the Solid Properties module in a MOOSE object, the source
code of the object must include the following lines of code.

Suppose we would like to use the thermal conductivity in a
kernel named `HeatDiffusion` that inherits from the `Kernel` class.
You simply need to add the desired material properties to the class members:

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=parameters end=getMaterialProperty

Once your MOOSE object inherits from the `DerivativeMaterialInterface` class, you can now
use material properties provided by the Solid Properties module. In the header file of the
MOOSE object where a solid material property is needed (in this case, the
thermal conductivity), `const` references to the material properties are required:

!listing modules/solid_properties/test/include/kernels/HeatDiffusion.h start=/// thermal end=};

The name `_k` is arbitrary, and may be selected as desired per application.

The desired material properties are then obtained in the constructor in the source file
according to the name of those properties defined in [ThermalSolidPropertiesMaterial](/materials/ThermalSolidPropertiesMaterial.md) (which
may be customized to be different from `k_solid`, `rho_solid`, and `cp_solid`). Here, the
default name of `"k_solid"` is used.

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=parameters end={

Then, material properties are used just as other MOOSE materials are used. For example, the
weak residual for the heat diffusion kernel grabs the value of thermal conductivity at the
present quadrature point.

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=computeQpResidual end=Real

### Input file syntax

The Solid Properties Materials are implemented in an input file in the `UserObjects` block.  For
example, to use stainless steel 316 thermal properties to provide the thermal conductivity in
the `HeatDiffusion` kernel above, the input file syntax would be:

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i
  start=UserObjects
  end=Kernels

Due to the consistent interface for solid properties, a different solid can be substituted in the
input file be changing the type of the userobject. For example, to set thermal properties
with an arbitrary functional dependence instead, the solid property module section of
the input file is:

!listing modules/solid_properties/test/tests/functional/test.i
  start=UserObjects
  end=Kernels

## Creating additional solids

New solids can be added to the Solid Properties module by inheriting from the base class appropriate
to the formulation and overriding the methods that describe the solid properties. New solids can also
be added to separate applications for more custom usage. These can then be
used in an identical manner as all other solid property userobjects.

For example, suppose new thermal solid properties were desired for a material named `GorillaGlue` in
the Navier-Stokes module. Begin from an empty material inheriting from [ThermalSolidProperties](/userobjects/ThermalSolidProperties.md).
Override all methods in [ThermalSolidProperties](/userobjects/ThermalSolidProperties.md) that you would like to imlement custom `GorillaGlue`
properties for. The header file for your new material indicates all methods that will be defined.

!listing language=cpp
#pragma once
//
#include "ThermalSolidProperties.h"
//
class ThermalGorillaGlueProperties : public ThermalSolidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();
//
  ThermalGorillaGlueProperties(const InputParameters & parameters);
//
  virtual Real k_from_T(const Real & T) const override;
  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;
  virtual void k_from_T(const DualReal & T, DualReal & k, DualReal & dk_dT) const override;
//
  virtual Real cp_from_T(const Real & T) const override;
  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;
  virtual void cp_from_T(const DualReal & T, DualReal & cp, DualReal & dcp_dT) const override;
//
  virtual Real rho_from_T(const Real & T) const override;
  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;
  virtual void rho_from_T(const DualReal & T, DualReal & rho, DualReal & drho_dT) const override;
};

Next, implement those methods in a `ThermalGorillaGlueProperties` userobject. You can provide
a class description to make it clear what properties this new material provides.

!listing language=cpp
#include "ThermalGorillaGlueProperties.h"
//
registerMooseObject("SolidPropertiesApp", ThermalGorillaGlueProperties);
//
InputParameters
ThermalGorillaGlueProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();
  params.addClassDescription("Stainless steel 316 thermal properties.");
  return params;
}
//
ThermalGorillaGlueProperties::ThermalGorillaGlueProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters)
{
}

Then, the remainder of the source file includes the material-specific implementations
of the methods defined in the header file for Gorilla Glue.

## Objects, Actions, and Syntax

!syntax complete groups=SolidPropertiesApp heading-level=3 actions=False
