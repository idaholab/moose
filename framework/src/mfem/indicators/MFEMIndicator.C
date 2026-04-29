//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMIndicator.h"
#include "MFEMProblem.h"

namespace Moose::MFEM
{
InputParameters
Indicator::validParams()
{
  InputParameters params = Object::validParams();
  params.registerBase("Indicator");
  params.registerSystemAttributeName("Indicator");

  params.addRequiredParam<VariableName>("variable", "Variable to perform amr with");
  params.addRequiredParam<std::string>("kernel", "Kernel to perform amr with");
  return params;
}

Indicator::Indicator(const InputParameters & params)
  : Object(params),
    _var_name(getParam<VariableName>("variable")),
    _kernel_name(getParam<std::string>("kernel")),
    _fespace(*getMFEMProblem().getGridFunction(_var_name)->ParFESpace())
{
}

} // namespace Moose::MFEM
#endif
