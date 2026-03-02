//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
template <typename UOBase, unsigned int state>
class MOOSEVariableToNEML2Tmpl : public MOOSEToNEML2Batched<Real, UOBase>
{
public:
  static InputParameters validParams();

  MOOSEVariableToNEML2Tmpl(const InputParameters & params);

#ifdef NEML2_ENABLED
protected:
  const MooseArray<Real> & elemMOOSEData() const override { return _moose_variable; }

  /// Coupled MOOSE variable to read data from
  const VariableValue & _moose_variable;
#endif
};

using MOOSEVariableToNEML2 = MOOSEVariableToNEML2Tmpl<ElementUserObject, 0>;
using MOOSEOldVariableToNEML2 = MOOSEVariableToNEML2Tmpl<ElementUserObject, 1>;
using MOOSEBoundaryVariableToNEML2 = MOOSEVariableToNEML2Tmpl<SideUserObject, 0>;
using MOOSEBoundaryOldVariableToNEML2 = MOOSEVariableToNEML2Tmpl<SideUserObject, 1>;
