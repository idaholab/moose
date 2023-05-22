//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplaceBoundaryBC.h"

registerMooseObject("NavierStokesApp", DisplaceBoundaryBC);

InputParameters
DisplaceBoundaryBC::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addClassDescription("For displacing a boundary");
  params.addRequiredCoupledVar("velocity", "The velocity at which to displace");
  params.addRequiredParam<unsigned short>(
      "component", "What component of velocity/displacement this object is acting on.");
  return params;
}

DisplaceBoundaryBC::DisplaceBoundaryBC(const InputParameters & parameters)
  : ADNodalBC(parameters),
    _velocity(adCoupledNodalValue<RealVectorValue>("velocity")),
    _u_old(_var.nodalValueOld()),
    _component(getParam<unsigned short>("component"))
{
}

ADReal
DisplaceBoundaryBC::computeQpResidual()
{
  return _u - (_u_old + this->_dt * _velocity(_component));
}
