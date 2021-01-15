//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVVariable.h"

class INSFVNoSlipWallBC;
class InputParameters;

class INSFVVelocityVariable : public INSFVVariable
{
public:
  INSFVVelocityVariable(const InputParameters & params);

  static InputParameters validParams();

#ifdef MOOSE_GLOBAL_AD_INDEXING
  using INSFVVariable::adGradSln;
  const VectorValue<ADReal> & adGradSln(const Elem * const elem) const override;
#endif
};
