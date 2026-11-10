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

/**
 * Linear spring law: one constant stiffness per local axis.
 */
class LinearSpring : public SpringLawBase
{
public:
  static InputParameters validParams();

  LinearSpring(const InputParameters & parameters);

protected:
  virtual void computeLaw(const RealVectorValue & deformation,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness) override;

  /// Stiffness along each local axis
  const RealVectorValue _stiffness_per_axis;
};
