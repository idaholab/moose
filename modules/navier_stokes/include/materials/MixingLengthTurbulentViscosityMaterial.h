//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

/*
 *Class to compute the total viscosity which considers molecular and
 *mixing length model turbulent viscosity.
 */
class MixingLengthTurbulentViscosityMaterial : public ADMaterial
{
public:
  static InputParameters validParams();

  MixingLengthTurbulentViscosityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const unsigned int _mesh_dimension;

  const ADVariableGradient & _grad_u;
  const ADVariableGradient & _grad_v;
  const ADVariableGradient & _grad_w;
  /// Turbulent eddy mixing length
  const VariableValue & _mixing_len;

  /// viscosity
  const ADMaterialProperty<Real> & _mu;

  /// density
  const ADMaterialProperty<Real> & _rho;
  // Total viscosity
  ADMaterialProperty<Real> & _total_viscosity;
};
