//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeNodalKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

InputParameters
TimeNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

TimeNodalKernel::TimeNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _u_dot(_var.dofValuesDot()), _du_dot_du(_var.dofValuesDuDotDu())
{
}
