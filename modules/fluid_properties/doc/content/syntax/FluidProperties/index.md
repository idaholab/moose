# FluidProperties System

## Overview

`FluidProperties` objects define interfaces for computing thermodynamic
properties of fluids (liquids and gases). The consistent interface allows
different fluids to be used in an input file by simply swapping the name of the
Fluid Properties UserObject in a plug-and-play manner.

There are multiple base classes suited to different phase and component
combinations, as well as to different applications:

- `HEMFluidProperties`: single component, single phase, HEM formulation
- `MultiComponentFluidProperties`: two components, single phase, $(p,T)$ formulation
- [SinglePhaseFluidProperties.md]: single component, single phase
- `TwoPhaseFluidProperties`: single component, two phases
- `VaporMixtureFluidProperties`: multiple components, single (vapor) phase

## Usage

Fluid properties objects are `GeneralUserObject`s that have empty `initialize()`, `execute()`
and `finalize()` methods, so they do nothing during a simulation. Their purpose is to provide
convenient access to fluid properties through the existing UserObject interface.

All Fluid Properties UserObjects can be accessed in MOOSE objects through the usual UserObject
interface. The following example provides a detailed explanation of the steps involved to use the
Fluid Properties UserObjects in other MOOSE objects, and the syntax required in the input file.

This example is for a problem that has energy-volume as the primary variables. A material is
provided to calculate fluid properties at the quadrature points.

### Source

To access the fluid properties defined in the Fluid Properties module in a MOOSE object, the
source code of the object must include the following lines of code.

In the header file of the material, a `const` reference to the base [SinglePhaseFluidProperties.md]
object is required:

!listing modules/fluid_properties/include/materials/FluidPropertiesMaterialVE.h line=SinglePhaseFluidProperties

!alert note
A forward declaration to the [SinglePhaseFluidProperties.md] class is required at the beginning of
the header file.

!listing modules/fluid_properties/include/materials/FluidPropertiesMaterialVE.h line=class SinglePhaseFluidProperties

In the source file, the `SinglePhaseFluidProperties` class must be included

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterialVE.C line= "SinglePhaseFluidProperties.h"

The Fluid Properties UserObject is passed to this material in the input file by adding a
UserObject name parameters in the input parameters:

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterialVE.C line=addRequiredParam

The reference to the UserObject is then initialized in the constructor using

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterialVE.C line=getUserObject

The properties defined in the Fluid Properties UserObject can now be accessed through the
reference. In this material, the `computeQpProperties` method calculates a number of properties
at the quadrature points using the values of `_v[_qp]` and `_e[_qp]`.

!listing modules/fluid_properties/src/materials/FluidPropertiesMaterialVE.C start=computeQpProperties

In a similar fashion, fluid properties can be accessed using the Automatic Differentiation interface
using the `DualReal` version which provides both the value and derivatives

```
DualReal rho = _fp.p_from_T_v(T, v);
```

where $T$ and $v$ are `DualReal`'s. The result (density `rho` in this example) then contains both the
value of density and its derivatives with respect to the primary variables `T` and `v`.

### Input file syntax

The Fluid Properties UserObjects are implemented in an input file in the `FluidProperties` block.  For
example, to use the ideal gas formulation for specific volume and energy, the input file syntax
would be:

!listing modules/fluid_properties/test/tests/ideal_gas/test.i block=FluidProperties

In this example, the user has specified a value for `gamma` (the ratio of isobaric to isochoric
specific heat capacities), and `R`, the universal gas constant.

The fluid properties can then be accessed by other MOOSE objects through the name given in the
input file.

!listing modules/fluid_properties/test/tests/ideal_gas/test.i block=Materials

Due to the consistent interface for fluid properties, a different fluid can be substituted in the
input file be changing the type of the UserObject. For example, to use a stiffened gas instead
of an ideal gas, the only modification required in the input file is

!listing modules/fluid_properties/test/tests/stiffened_gas/test.i block=FluidProperties

## Creating additional fluids

New fluids can be added to the Fluid Properties module by inheriting from the base class and
overriding the methods that describe the fluid properties. These can then be used in an
identical manner as all other Fluid Properties UserObjects.

## Utilities

### Fluid Properties Interrogator

The [FluidPropertiesInterrogator](/FluidPropertiesInterrogator.md) is a user
object which can be used to query eligible fluid properties objects.

### Fluid properties materials

The [FluidPropertiesMaterialVE.md] and [FluidPropertiesMaterialPT.md] are materials
which define many fluid properties as material properties, mainly for visualizing them over
the solve domain.

!syntax list /FluidProperties objects=True actions=False subsystems=False

!syntax list /FluidProperties objects=False actions=False subsystems=True

!syntax list /FluidProperties objects=False actions=True subsystems=False
