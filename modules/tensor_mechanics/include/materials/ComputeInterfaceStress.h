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
#include "RankTwoTensorForward.h"

/**
 * Calculates an Extra-Stress tensor that lies in the plane of an interface
 * defined by the gradient of an order parameter.
 */
class ComputeInterfaceStress : public Material
{
public:
  static InputParameters validParams();

  ComputeInterfaceStress(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  std::size_t _nvar;
  std::vector<const VariableGradient *> _grad_v;
  std::vector<Real> _op_range;
  std::vector<Real> _stress;

  MaterialProperty<RankTwoTensor> & _planar_stress;
};
