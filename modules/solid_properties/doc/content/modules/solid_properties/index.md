# Solid Properties Module

The Solid Properties module provides a consistent interface to solid properties. Unlike
fluid properties that are often times only a function of pressure and temperature
(or two other state variables), solid properties are frequently a function of many
other parameters in addition to temperature, such as accumulated radiation damage,
oxidation state, and microstructure. Some common properties include thermal
properties such as specific heat, thermal conductivity, and emissivity; and
mechanical properties such as elastic modulus and Poisson ratio. This module uses a
flexible design to permit extension by other applications to include dependencies on
application-specific parameters. In addition, a consistent interface exists to access
derivatives with respect to the primary variables. This module allows different solids
to be used in the input files by simply swapping the name of the Solid Properties
Material in a plug-and-play manner.

!alert warning
Always verify that the material properties provided in this module agree with
the system you are modeling. The selection of particular correlations here does not
constitute an endorsement of the accuracy of those correlations.

This module provides solid properties for different solid materials, organized according to
the overall type of property.

## Solid properties

All solid properties inherit from the `SolidPropertiesMaterial` base material class, which provides
interfaces for all types of solid properties that might be desired, in addition to:

- Solid name: `solidName()`

## Thermal properties

Thermal properties (density, specific heat, and thermal conductivity) are computed by
materials inheriting from the `ThermalSolidPropertiesMaterial` base class. This class
defines these three material properties and their derivatives with respect to temperature.
For flexibility, a custom name may be provided for each of these three properties.

In the `ThermalSolidProperties` material, the `computeQpProperties` method calculates a number of properties at
the quadrature points using the values of `_temperature[_qp]`.

!listing modules/solid_properties/src/materials/ThermalSolidPropertiesMaterial.C start=computeQpProperties

The calculation of material properties is divided into several different methods:

- compute isobaric specific heat - `computeIsobaricSpecificHeat()`
- compute derivatives of isobaric specific heat - `computeIsobaricSpecificHeatDerivatives()`
- compute thermal conductivity - `computeThermalConductivity()`
- compute derivatives of thermal conductivity - `computeThermalConductivityDerivatives()`
- compute density - `computeDensity()`
- compute derivatives of density - `computeDensityDerivatives()`

The methods for computing derivatives are designed such that no value is returned (return
type is `void` to accomodate applications needing multiple derivatives of a property, such
as derivatives of thermal conductivity with respect to temperature, strain, and density).
An example is provided below for creating a new solid material providing thermal properties.

Materials available in the SolidProperties module that provide thermal properties
are:

- [functional](/ThermalFunctionSolidProperties.md)
- [graphite](/ThermalGraphiteProperties.md)
- [silicon carbide](/ThermalSiliconCarbideProperties.md)
- [stainless steel alloy 316](/ThermalStainlessSteel316Properties.md)

!alert note
Additional classes of solid properties, such as mechanical properties, are not yet
implemented.

## Usage

All Solid Properties materials can be accessed in MOOSE objects through the usual Material
interface. The following example provides a detailed explanation of the steps involved to use the
Solid Properties Materials in other MOOSE objects, and the syntax required in the input file.

This example is for a problem where thermal conductivity and its derivative are needed in a kernel.
An example input file syntax using this kernel and the Solid Properties module is also provided.
Similar steps would be followed for other material properties.

### Source

To access the solid properties defined in the Solid Properties module in a MOOSE object, the source
code of the object must include the following lines of code.

To provide the consistent interface to solid material property derivatives, the kernel must
inherit from the templated `DerivativeMaterialInterface` class. If your MOOSE object already
inherits from this class, then skip forward to the discussion on how to retrieve solid
material properties. Assuming your MOOSE object does not already inherit from the
`DerivativeMaterialInterface` class, continue here.

Suppose we would like to use the thermal conductivity and its derivative in a
kernel named `HeatDiffusion` that inherits
from the `Kernel` class. Simply modify the inheritance in the header file to the templated
`DerivativeMaterialInterface` class:

!listing modules/solid_properties/test/include/kernels/HeatDiffusion.h line=DerivativeMaterialInterface<Kernel>

!alert note
An include statement to the `DerivativeMaterialInterface` class is required at the beginning of the
header file.

!listing modules/solid_properties/test/include/kernels/HeatDiffusion.h line=include

Finally, in the source file of the MOOSE object where a solid material property is needed,
modify the constructor to obtain the parameters of the templated base class:

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=parameters end=getMaterialProperty

Once your MOOSE object inherits from the `DerivativeMaterialInterface` class, you can now
use material properties provided by the Solid Properties module. In the header file of the
MOOSE object where a solid material property is needed (in this case, the
thermal conductivity and its derivative), `const` references to the material properties are required:

!listing modules/solid_properties/test/include/kernels/HeatDiffusion.h start=/// thermal end=};

The names `_k` and `_dk_dT` are arbitrary, and may be selected as desired per application.

The desired material properties are then obtained in the constructor in the source file
according to the name of those properties defined in `ThermalSolidPropertiesMaterial` (which
may be customized to be different from `k_solid`, `rho_solid`, and `cp_solid`). Here, the
default name of `"k_solid"` is used.

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=parameters end={

Then, material properties are used just as other MOOSE materials are used. For example, the
weak residual for the heat diffusion kernel grabs the value of thermal conductivity at the
present quadrature point.

!listing modules/solid_properties/test/src/kernels/HeatDiffusion.C start=computeQpResidual end=Real

### Input file syntax

The Solid Properties Materials are implemented in an input file in the `Materials` block.  For
example, to use stainless steel 316 thermal properties to provide the thermal conductivity in
the `HeatDiffusion` kernel above, the input file syntax would be:

!listing modules/solid_properties/test/tests/stainless_steel_316/test.i block=Materials

Due to the consistent interface for solid properties, a different solid can be substituted in the
input file be changing the type of the Material. For example, to set thermal properties
with an arbitrary functional dependence instead, the solid property module section of
the input file is:

!listing modules/solid_properties/test/tests/functional/test.i block=Materials

## Creating additional solids

New solids can be added to the Solid Properties module by inheriting from the base class appropriate
to the formulation and overriding the methods that describe the solid properties. New solids can also
be added to separate applications for more custom usage. These can then be
used in an identical manner as all other Solid Properties Materials.

For example, suppose new thermal solid properties were desired for a material named `GorillaGlue` in
the Navier-Stokes module. Begin from an empty material inheriting from `ThermalSolidPropertiesMaterial`.
Override all methods in `SolidPropertiesMaterial` that you would like to imlement custom `GorillaGlue`
properties for. The header file for your new material indicates all methods that will be defined.

!listing language=cpp
#ifndef THERMALGORILLAGLUEMATERIAL_H
#define THERMALGORILLAGLUEMATERIAL_H
//
#include "ThermalSolidPropertiesMaterial.h"
//
class ThermalGorillaGlueProperties;
template <> InputParameters validParams<ThermalGorillaGlueProperties>();
//
class ThermalGraphiteProperties : public ThermalSolidPropertiesMaterial
{
public:
  ThermalGraphiteProperties(const InputParameters & parameters);
  virtual const std::string & solidName() const override;
  virtual Real molarMass() const override;
  virtual void computeIsobaricSpecificHeat() override;
  virtual void computeIsobaricSpecificHeatDerivatives() override;
  virtual void computeThermalConductivity() override;
  virtual void computeThermalConductivityDerivatives() override;
  virtual void computeDensity() override;
  virtual void computeDensityDerivatives() override;
private:
  static const std::string _name;
};
#endif /* THERMALGORILLAGLUEMATERIAL_H */

Next, implement those methods in a `ThermalGorillaGlueProperties` material. You can provide
a class description to make it clear what properties this new material provides.

!listing language=cpp
#include "ThermalGorillaGlueProperties.h"
registerMooseObject("SolidPropertiesApp", ThermalGorillaGlueProperties);
const std::string ThermalGorillaGlueProperties::_name = std::string("thermal_gorilla_glue");
//
template <>
InputParameters
validParams<ThermalGorillaGlueProperties>()
{
  InputParameters params = validParams<ThermalSolidPropertiesMaterial>();
  params.addClassDescription("ThermalSolidPropertiesMaterial defining Gorilla Glue properties.");
  return params;
}
//
ThermalGorillaGlueProperties::ThermalGorillaGlueProperties(
    const InputParameters & parameters)
  : ThermalSolidPropertiesMaterial(parameters)
{
}

Then, the remainder of the source file includes the material-specific implementations
of the methods defined in the header file for Gorilla Glue.

## Objects, Actions, and Syntax

!syntax complete group=SolidPropertiesApp level=3 actions=False
