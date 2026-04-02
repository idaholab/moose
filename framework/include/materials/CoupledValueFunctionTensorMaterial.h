//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * Assembles a RankTwoTensor from scalar material properties or constants
 */
class CoupledValueFunctionTensorMaterial : public Material
{
public:
  static InputParameters validParams();
  CoupledValueFunctionTensorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// RankTwoTensor material property
  MaterialProperty<RankTwoTensor> & _prop;

  std::vector<const MaterialProperty<Real> *> _mat_props;
  std::vector<Real> _const_vals;
  std::vector<bool> _is_const;
};
