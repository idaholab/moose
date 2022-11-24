//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMTimeKernel.h"
#include "MooseVariableFE.h"

InputParameters
LMTimeKernel::validParams()
{
  auto params = LMKernel::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  return params;
}

LMTimeKernel::LMTimeKernel(const InputParameters & parameters)
  : LMKernel(parameters), _u_dot(_var.adUDot())
{
}
