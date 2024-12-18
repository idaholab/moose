//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2Batched.h"

/**
 * Gather a MOOSE variable for insertion into the NEML2 model.
 */
template <unsigned int state>
class MOOSEVariableToNEML2Templ : public MOOSEToNEML2Batched<Real>
{
public:
  static InputParameters validParams();

  MOOSEVariableToNEML2Templ(const InputParameters & params);

#ifdef NEML2_ENABLED
protected:
  const MooseArray<Real> & elemMOOSEData() const override { return _moose_variable; }

  /// Coupled MOOSE variable to read data from
  const VariableValue & _moose_variable;
#endif
};

using MOOSEVariableToNEML2 = MOOSEVariableToNEML2Templ<0>;
using MOOSEOldVariableToNEML2 = MOOSEVariableToNEML2Templ<1>;
