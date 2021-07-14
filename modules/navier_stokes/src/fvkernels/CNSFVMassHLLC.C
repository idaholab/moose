//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVMassHLLC.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVMassHLLC);

InputParameters
CNSFVMassHLLC::validParams()
{
  InputParameters params = CNSFVHLLC::validParams();
  params.addClassDescription(
      "Implements the mass flux portion of the free-flow HLLC discretization.");
  return params;
}

CNSFVMassHLLC::CNSFVMassHLLC(const InputParameters & params) : CNSFVHLLC(params) {}

ADReal
CNSFVMassHLLC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp];
}

ADReal
CNSFVMassHLLC::fluxNeighbor()
{
  return _normal_speed_neighbor * _rho_neighbor[_qp];
}

ADReal
CNSFVMassHLLC::hllcElem()
{
  return 1;
}

ADReal
CNSFVMassHLLC::hllcNeighbor()
{
  return 1;
}

ADReal
CNSFVMassHLLC::conservedVariableElem()
{
  return _rho_elem[_qp];
}

ADReal
CNSFVMassHLLC::conservedVariableNeighbor()
{
  return _rho_neighbor[_qp];
}
