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

/**
 * ComputeEigenstrainTrussBase is the base class for beam eigenstrain vectors
 */
class ComputeEigenstrainTrussBase : public Material
{
public:
  static InputParameters validParams();

  ComputeEigenstrainTrussBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// Compute the eigenstrain and store in _disp_eigenstrain and _rot_eigenstrain
  virtual void computeQpEigenstrain() = 0;

  /// Base material property name for the eigenstrain vectors
  std::string _eigenstrain_name;

  /// Stores the current displacement eigenstrain
  MaterialProperty<RealVectorValue> & _disp_eigenstrain;

  /// Restartable data to check for the zeroth and first time steps for thermal calculations
  bool & _step_zero;
};
