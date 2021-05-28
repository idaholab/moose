//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCMassStagnationInletBC.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVHLLCMassStagnationInletBC);

InputParameters
CNSFVHLLCMassStagnationInletBC::validParams()
{
  InputParameters params = CNSFVHLLCStagnationInletBC::validParams();
  params.addClassDescription(
      "Adds the boundary mass flux for HLLC when provided stagnation temperature and pressure");
  return params;
}

CNSFVHLLCMassStagnationInletBC::CNSFVHLLCMassStagnationInletBC(const InputParameters & parameters)
  : CNSFVHLLCStagnationInletBC(parameters)
{
}

ADReal
CNSFVHLLCMassStagnationInletBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp];
}

ADReal
CNSFVHLLCMassStagnationInletBC::fluxBoundary()
{
  return _normal_speed_boundary * _rho_boundary;
}

ADReal
CNSFVHLLCMassStagnationInletBC::hllcElem()
{
  return 1;
}

ADReal
CNSFVHLLCMassStagnationInletBC::hllcBoundary()
{
  return 1;
}

ADReal
CNSFVHLLCMassStagnationInletBC::conservedVariableElem()
{
  return _rho_elem[_qp];
}

ADReal
CNSFVHLLCMassStagnationInletBC::conservedVariableBoundary()
{
  return _rho_boundary;
}
