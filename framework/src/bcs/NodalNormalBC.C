//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalBC.h"

InputParameters
NodalNormalBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addCoupledVar("nx", "x-component of the normal");
  params.addCoupledVar("ny", "y-component of the normal");
  params.addCoupledVar("nz", "z-component of the normal");

  params.set<std::vector<VariableName>>("nx") = {"nodal_normal_x"};
  params.set<std::vector<VariableName>>("ny") = {"nodal_normal_y"};
  params.set<std::vector<VariableName>>("nz") = {"nodal_normal_z"};

  return params;
}

NodalNormalBC::NodalNormalBC(const InputParameters & parameters)
  : NodalBC(parameters), _nx(coupledValue("nx")), _ny(coupledValue("ny")), _nz(coupledValue("nz"))
{
}

void
NodalNormalBC::computeResidual()
{
  _normal = Point(_nx[_qp], _ny[_qp], _nz[_qp]);
  computeQpResidual();
}
