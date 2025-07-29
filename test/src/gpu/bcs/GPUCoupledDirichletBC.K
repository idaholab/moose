//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUCoupledDirichletBC.h"

registerMooseObject("MooseTestApp", KokkosCoupledDirichletBC);

InputParameters
KokkosCoupledDirichletBC::validParams()
{
  InputParameters params = KokkosDirichletBC::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable");
  return params;
}

KokkosCoupledDirichletBC::KokkosCoupledDirichletBC(const InputParameters & parameters)
  : KokkosDirichletBC(parameters), _v(kokkosCoupledNodalValue("v")), _v_num(coupled("v")), _c(1.0)
{
}
