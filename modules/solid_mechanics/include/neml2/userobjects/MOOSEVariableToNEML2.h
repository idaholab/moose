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
NEML2ObjectStubHeader(MOOSEOldVariableToNEML2, MOOSEToNEML2);
#else

/**
 * Gather a MOOSE variable for insertion into the specified input of a NEML2 model.
 */
template <unsigned int state>
class MOOSEVariableToNEML2Templ : public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  MOOSEVariableToNEML2Templ(const InputParameters & params);

protected:
  virtual torch::Tensor convertQpMOOSEData() const override;

  /// Coupled MOOSE variable to read data from
  const VariableValue & _moose_variable;
};

typedef MOOSEVariableToNEML2Templ<0> MOOSEVariableToNEML2;
typedef MOOSEVariableToNEML2Templ<1> MOOSEOldVariableToNEML2;

#endif
