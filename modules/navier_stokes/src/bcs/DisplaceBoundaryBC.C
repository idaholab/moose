/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#include "DisplaceBoundaryBC.h"

registerMooseObject("MalamuteApp", DisplaceBoundaryBC);

InputParameters
DisplaceBoundaryBC::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addClassDescription("For displacing a boundary");
  params.addRequiredCoupledVar("velocity", "The velocity at which to displace");
  return params;
}

DisplaceBoundaryBC::DisplaceBoundaryBC(const InputParameters & parameters)
  : ADNodalBC(parameters),
    _velocity(adCoupledNodalValue<Real>("velocity")),
    _u_old(_var.nodalValueOld())
{
}

ADReal
DisplaceBoundaryBC::computeQpResidual()
{
  return _u - (_u_old + this->_dt * _velocity);
}
