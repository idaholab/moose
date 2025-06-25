//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMatchedValueBC.h"

registerMooseObject("MooseApp", KokkosMatchedValueBC);

InputParameters
KokkosMatchedValueBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addRequiredCoupledVar("v", "The variable whose value we are to match.");
  params.addParam<Real>("u_coeff", 1.0, " A coefficient for primary variable u");
  params.addParam<Real>("v_coeff", 1.0, " A coefficient for coupled variable v");
  params.addClassDescription("Implements a NodalBC which equates two different Variables' values "
                             "on a specified boundary.");
  return params;
}

KokkosMatchedValueBC::KokkosMatchedValueBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _v(KokkosCoupledNodalValue("v")),
    _v_num(coupled("v")),
    _u_coeff(getParam<Real>("u_coeff")),
    _v_coeff(getParam<Real>("v_coeff"))
{
}
