//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultConvergenceBase.h"

InputParameters
DefaultConvergenceBase::validParams()
{
  InputParameters params = Convergence::validParams();

  params.addPrivateParam<bool>("added_as_default", false);

  return params;
}

DefaultConvergenceBase::DefaultConvergenceBase(const InputParameters & parameters)
  : Convergence(parameters), _added_as_default(getParam<bool>("added_as_default"))
{
}

void
DefaultConvergenceBase::initialSetup()
{
  Convergence::initialSetup();

  checkDuplicateSetSharedExecutionerParams();
}

void
DefaultConvergenceBase::checkDuplicateSetSharedExecutionerParams() const
{
  if (_duplicate_shared_executioner_params.size() > 0 && !_added_as_default)
  {
    std::ostringstream oss;
    oss << "The following parameters were set in both this Convergence object and the "
           "executioner:\n";
    for (const auto & param : _duplicate_shared_executioner_params)
      oss << "  " << param << "\n";
    mooseError(oss.str());
  }
}
