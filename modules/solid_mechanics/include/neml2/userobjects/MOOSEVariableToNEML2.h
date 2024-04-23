//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubHeader(MOOSEVariableToNEML2, MOOSEToNEML2);
#else

/**
 * Gather a MOOSE variable for insertion into the specified input of a NEML2 model.
 */
class MOOSEVariableToNEML2 : public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  MOOSEVariableToNEML2(const InputParameters & params);

protected:
  virtual void execute() override;

  const VariableValue & _moose_variable;
};

#endif
