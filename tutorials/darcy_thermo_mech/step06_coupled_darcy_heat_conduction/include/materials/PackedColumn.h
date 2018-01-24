//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PACKEDCOLUMN_H
#define PACKEDCOLUMN_H

#include "Material.h"

// A helper class from MOOSE that linear interpolates x,y data
#include "LinearInterpolation.h"

class PackedColumn;

template <>
InputParameters validParams<PackedColumn>();

/**
 * Material-derived objects override the computeQpProperties()
 * function.  They must declare and compute material properties for
 * use by other objects in the calculation such as Kernels and
 * BoundaryConditions.
 */
class PackedColumn : public Material
{
public:
  PackedColumn(const InputParameters & parameters);

protected:
  /**
   * Necessary override.  This is where the values of the properties
   * are computed.
   */
  virtual void computeQpProperties() override;

  /// The radius of the spheres in the column
  const Real & _sphere_radius;

  /// The permeability of the medium is based on a linear
  /// interpolation between the two different sphere sizes which are
  /// assumed to be present.
  LinearInterpolation _permeability_interpolation;

  /// The permeability (K)
  MaterialProperty<Real> & _permeability;

  /// The porosity (eps)
  MaterialProperty<Real> & _porosity;

  /// The viscosity of the fluid (mu)
  MaterialProperty<Real> & _viscosity;

  /// The bulk thermal conductivity
  MaterialProperty<Real> & _thermal_conductivity;

  /// The bulk heat capacity
  MaterialProperty<Real> & _heat_capacity;

  /// The bulk density
  MaterialProperty<Real> & _density;

  /// Single value to store the interpolated permeability base on
  /// sphere size.  The _sphere_radius is assumed to be constant, so
  /// we only have to compute this once.
  Real _interpolated_permeability;
};

#endif // PACKEDCOLUMN_H
