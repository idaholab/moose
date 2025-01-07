//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsScalarTransferBase.h"

/**
 * Scalar transfer connection using Physics and functors to describe the wall scalar values
 */
class PhysicsScalarTransferFromFunctors : public PhysicsScalarTransferBase
{
public:
  static InputParameters validParams();

  PhysicsScalarTransferFromFunctors(const InputParameters & parameters);

  void init() override;

  const std::vector<MooseFunctorName> & getWallScalarValuesNames() const override
  {
    return _wall_scalar_functors;
  }

protected:
  /// Names of the functors providing the wall values of the scalar variables
  const std::vector<MooseFunctorName> _wall_scalar_functors;
};
