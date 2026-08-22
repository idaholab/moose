//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVSetupTester.h"
#include "FVDirichletBCBase.h"

/**
 * FV Dirichlet boundary condition that counts the setup methods it receives, for testing setup
 * dispatch.
 */
class FVSetupTestDirichletBC : public FVSetupTester<FVDirichletBCBase>
{
public:
  static InputParameters validParams();
  FVSetupTestDirichletBC(const InputParameters & params);

  virtual ADReal boundaryValue(const FaceInfo &, const Moose::StateArg &) const override
  {
    return _value;
  }

protected:
  /// The value this boundary condition imposes
  const Real _value;
};
