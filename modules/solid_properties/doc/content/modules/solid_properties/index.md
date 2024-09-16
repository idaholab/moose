# Solid Properties Module

The Solid Properties module provides a consistent interface to solid properties. Unlike
fluid properties that are oftentimes only a function of pressure and temperature
(or two other state variables), solid properties are frequently a function of many
other parameters in addition to temperature, such as accumulated radiation damage,
oxidation state, and microstructure. Some commonly-used properties in physics
simulations include thermal
properties such as specific heat, thermal conductivity, and emissivity; and
mechanical properties such as elastic modulus and Poisson ratio.

This module provides a consistent interface to define solid material properties.
A flexible design permits extension by other applications to include custom
properties. This module allows different solids
to be used in a physics simulation by simply swapping the name of the solid properties
userobject in a plug-and-play manner.

!alert warning
Always verify that the material properties provided in this module agree with
the system you are modeling. The selection of particular correlations here does not
constitute an endorsement of the accuracy of those correlations.

This module provides solid properties for different solids, organized according to
the overall type of property. The overall design of this module is as follows:

1. *UserObjects* contain functions to compute various solid properties in terms
   of dependent quantities (temperature, porosity, etc.). Different "categories" of
   solid properties are grouped together into different userobjects. For example,
   common solid properties used in thermal simulations (specific heat, thermal conductivity),
   are provided by derived classes of the [ThermalSolidProperties](/solidproperties/ThermalSolidProperties.md)
   userobject. Mechanics properties (Young's modulus, Poisson ratio), would be provided by
   a separate "family" of userobjects.
2. *Materials* call these userobject functions to compute the material properties at
   quadrature points. Like the userobjects, these materials are grouped into "categories."
   For example, common solid properties for thermal simulations are evaluated by the
   [ThermalSolidPropertiesMaterial](/materials/ThermalSolidPropertiesMaterial.md).

## Thermal properties

Thermal properties (density, specific heat, and thermal conductivity) are computed by
userobjects inheriting from the [ThermalSolidProperties](/solidproperties/ThermalSolidProperties.md) base class. This class
defines functions to compute these properties as a function of temperature:

- compute isobaric specific heat - `Real cp_from_T(const Real & T)`
- compute thermal conductivity - `Real k_from_T(const Real & T)`
- compute density - `Real rho_from_T(const Real & T)`

Functions to compute derivatives of these properties as a function of temperature
are also available:

- compute isobaric specific heat and its temperature derivative - `void cp_from_T(const Real & T, Real & cp, Real & dcp_dT)`
- compute thermal conductivity and its temperature derivative - `void k_from_T(const Real & T, Real & k, Real & dk_dT)`
- compute density and its temperature derivative - `void rho_from_T(const Real & T, Real & rho, Real & drho_dT)`

Userobjects available in the Solid Properties module that provide thermal properties are:

- [graphite](/ThermalGraphiteProperties.md)
- [monolithic silicon carbide](/ThermalMonolithicSiCProperties.md)
- [composite silicon carbide](/ThermalCompositeSiCProperties.md)
- [stainless steel alloy 316](/ThermalSS316Properties.md)
- [using functions](/ThermalFunctionSolidProperties.md)

An example will be provided later on this page for creating a new solid userobject.

On their own, these userobjects do not execute; their functions must be called from other
objects. Some potentially useful classes that call them are:

- [ThermalSolidPropertiesFunctorMaterial.md]: A functor material that declares
  functor material properties for density, thermal conductivity, isobaric specific heat,
  and specific internal energy. An option is provided for using a constant density.
  This functor material can have its functor material properties converted to
  regular AD or non-AD material properties by using it in conjunction with
  [(AD)MaterialFunctorConverter](MaterialFunctorConverter.md).
- [ThermalSolidPropertiesMaterial.md] and [ConstantDensityThermalSolidPropertiesMaterial.md],
  which declare AD or non-AD material properties for density, thermal conductivity, and isobaric specific heat,
  using variable density and constant density, respectively.
- [ThermalSolidPropertiesPostprocessor.md] evaluates density, thermal conductivity,
  or isobaric specific heat at a single temperature value.

## Usage

The solid properties material can be accessed in MOOSE objects through the usual Material
interface. Here, we show an example where thermal conductivity is needed in a kernel
named `HeatDiffusion`.

### Source

To access the properties defined in the Solid Properties module in a MOOSE object,
add the desired material properties to the class members:

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=parameters end={

In the header file of the MOOSE object where a solid material property is needed,
`const` references to the material properties are required:

!listing modules/solid_properties/test/include/kernels/HeatDiffusion.h start=/// thermal end=};

The name `_k` is arbitrary, and may be selected as desired per application.
Then, material properties are used just as other MOOSE materials are used. For example, the
weak residual for the heat diffusion kernel grabs the value of thermal conductivity at the
present quadrature point.

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=computeQpResidual end=Real

### Input file syntax

The userobjects defining the material properties are set up in the `UserObjects` block.  For
example, to use stainless steel 316 thermal properties to provide the thermal conductivity in
the `HeatDiffusion` kernel above, the input file syntax would be:

```
[Modules]
  [SolidProperties]
    [steel]
      type = ThermalSS316Properties
    []
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSolidPropertiesMaterial
    temperature = T
    sp = steel
  []
[]
```

Due to the consistent interface for solid properties, a different solid can be substituted in the
input file by changing the type of the userobject. For example, to set thermal properties
with a general functional dependence instead, the solid property module section of
the input file is:

```
[Modules]
  [SolidProperties]
    [func]
      type = ThermalFunctionSolidProperties
      rho = '1000.0'
      cp = '200*t+150.0'
      k = '2.0*exp(-100.0/(2.0+t))+1.0'
    []
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSolidPropertiesMaterial
    sp = func
    temperature = u
  []
[]
```

## Creating additional solids

New solids can be added by inheriting from the userobject base class appropriate
to the formulation and overriding the methods that describe the solid properties.

For example, suppose new thermal solid properties were desired for granite.
The first step is to create a userobject named `ThermalGraniteProperties` by
inheriting from [ThermalSolidProperties](/solidproperties/ThermalSolidProperties.md) and
overriding all methods that you would like to implement.
The header file for your new userobject indicates all methods that will be defined.

!listing! language=cpp
#pragma once

#include "ThermalSolidProperties.h"

class ThermalGraniteProperties : public ThermalSolidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();

  ThermalGraniteProperties(const InputParameters & parameters);

  virtual Real k_from_T(const Real & T) const override;
  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;
  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;
  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;
};

#pragma GCC diagnostic pop
!listing-end!

Next, implement those methods in a `ThermalGraniteProperties` userobject. You can provide
a class description to make it clear what properties this new material provides.

!listing! language=cpp
#include "ThermalGraniteProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalGraniteProperties);

InputParameters
ThermalGraniteProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();
  params.addClassDescription("Granite thermal properties.");
  return params;
}

ThermalGraniteProperties::ThermalGraniteProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters)
{
}
!listing-end!

Then, the remainder of the source file includes the material-specific implementations
of the methods defined in the header file for granite.

## Objects, Actions, and Syntax

!syntax complete groups=SolidPropertiesApp heading-level=3 actions=False
