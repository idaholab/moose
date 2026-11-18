//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpringLawBase.h"

class Function;

/**
 * Nonlinear elastic spring law: the force along each local axis is a function of the
 * deformation along that axis, loading and unloading on the same curve.
 */
class NonlinearElasticSpring : public SpringLawBase
{
public:
  static InputParameters validParams();

  NonlinearElasticSpring(const InputParameters & parameters);

protected:
  virtual void computeLaw(const RealVectorValue & deformation,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness) override;

  /// Force-deformation curve of each local axis, a function of the x coordinate
  std::vector<const Function *> _backbones;
};
