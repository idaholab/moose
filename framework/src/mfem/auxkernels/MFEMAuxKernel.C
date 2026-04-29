//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMAuxKernel.h"
#include "MFEMProblem.h"

namespace Moose::MFEM
{
InputParameters
AuxKernel::validParams()
{
  InputParameters params = ExecutedObject::validParams();
  params.registerBase("AuxKernel");
  params.addClassDescription("Base class for MFEM objects that update auxiliary variables outside "
                             "of the main solve step.");
  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this object applies to");
  return params;
}

AuxKernel::AuxKernel(const InputParameters & parameters)
  : ExecutedObject(parameters),
    _result_var_name(getParam<AuxVariableName>("variable")),
    _result_var(*getMFEMProblem().getGridFunction(_result_var_name))
{
}

std::optional<std::string>
AuxKernel::suppliedVariableName() const
{
  return _result_var_name;
}

} // namespace Moose::MFEM
#endif
