//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDNodalBC.h"

InputParameters
OneDNodalBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  return params;
}

OneDNodalBC::OneDNodalBC(const InputParameters & parameters)
  : NodalBC(parameters), _normal(getParam<Real>("normal"))
{
}
