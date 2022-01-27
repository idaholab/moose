//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

// A helper class from MOOSE that linear interpolates x,y data
#include "LinearInterpolation.h"

/**
 * Material-derived objects override the computeQpProperties()
 * function.  They must declare and compute material properties for
 * use by other objects in the calculation such as Kernels and
 * BoundaryConditions.
 */
class PackedColumn : public Material
{
public:
  static InputParameters validParams();

  PackedColumn(const InputParameters & parameters);

protected:
  /**
   * Necessary override.  This is where the values of the properties
   * are computed.
   */
  virtual void computeQpProperties() override;

  /**
   * Helper function for reading CSV data for use in an interpolator object.
   */
  bool initInputData(const std::string & param_name, ADLinearInterpolation & interp);

  /// The radius of the spheres in the column
  const Function & _input_radius;

  /// The input porosity
  const Function & _input_porosity;

  /// Temperature
  const ADVariableValue & _temperature;

  /// Compute permeability based on the radius (mm)
  LinearInterpolation _permeability_interpolation;

  /// Fluid viscosity
  bool _use_fluid_mu_interp;
  const Real & _fluid_mu;
  ADLinearInterpolation _fluid_mu_interpolation;

  /// Fluid thermal conductivity
  bool _use_fluid_k_interp = false;
  const Real & _fluid_k;
  ADLinearInterpolation _fluid_k_interpolation;

  /// Fluid density
  bool _use_fluid_rho_interp = false;
  const Real & _fluid_rho;
  ADLinearInterpolation _fluid_rho_interpolation;

  /// Fluid specific heat
  bool _use_fluid_cp_interp;
  const Real & _fluid_cp;
  ADLinearInterpolation _fluid_cp_interpolation;

  /// Fluid thermal expansion coefficient
  bool _use_fluid_cte_interp;
  const Real & _fluid_cte;
  ADLinearInterpolation _fluid_cte_interpolation;

  /// Solid thermal conductivity
  bool _use_solid_k_interp = false;
  const Real & _solid_k;
  ADLinearInterpolation _solid_k_interpolation;

  /// Solid density
  bool _use_solid_rho_interp = false;
  const Real & _solid_rho;
  ADLinearInterpolation _solid_rho_interpolation;

  /// Solid specific heat
  bool _use_solid_cp_interp;
  const Real & _solid_cp;
  ADLinearInterpolation _solid_cp_interpolation;

  /// Solid thermal expansion coefficient
  bool _use_solid_cte_interp;
  const Real & _solid_cte;
  ADLinearInterpolation _solid_cte_interpolation;

  /// The permeability (K)
  ADMaterialProperty<Real> & _permeability;

  /// The porosity (eps)
  ADMaterialProperty<Real> & _porosity;

  /// The viscosity of the fluid (mu)
  ADMaterialProperty<Real> & _viscosity;

  /// The bulk thermal conductivity
  ADMaterialProperty<Real> & _thermal_conductivity;

  /// The bulk heat capacity
  ADMaterialProperty<Real> & _specific_heat;

  /// The bulk density
  ADMaterialProperty<Real> & _density;

  /// The bulk thermal expansion coefficient
  ADMaterialProperty<Real> & _thermal_expansion;
};
