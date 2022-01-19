#pragma once

#include "Material.h"

// A helper class from MOOSE that linearly interpolates abscissa-ordinate pairs
#include "LinearInterpolation.h"

/**
 * Computes the permeability of a porous medium made up of packed steel spheres of a specified
 * diameter in accordance with Pamuk and Ozdemir (2012). This also provides a specified dynamic
 * viscosity of the fluid in the medium.
 */
class PackedColumn : public Material
{
public:
  static InputParameters validParams();

  PackedColumn(const InputParameters & parameters);

protected:
  /// Necessary override. This is where the property values are set.
  virtual void computeQpProperties() override;

  /// The inputs for the diameter of spheres in the column and the dynamic viscosity of the fluid
  const Real & _diameter;
  const Real & _input_viscosity;

  /// This object interpolates permeability (m^2) based on the diameter (mm)
  LinearInterpolation _permeability_interpolation;

  /// The material property objects that hold values for permeability (K) and dynamic viscosity (mu)
  ADMaterialProperty<Real> & _permeability;
  ADMaterialProperty<Real> & _viscosity;
};
