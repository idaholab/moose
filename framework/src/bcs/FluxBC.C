/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FluxBC.h"


template<>
InputParameters validParams<FluxBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}


FluxBC::FluxBC(const std::string & name, InputParameters params) :
    IntegratedBC(name, params)
{
}

FluxBC::~FluxBC()
{
}

Real
FluxBC::computeQpResidual()
{
  return computeQpFluxResidual() * _normals[_qp] * _test[_i][_qp];
}

Real
FluxBC::computeQpJacobian()
{
  return computeQpFluxJacobian() * _normals[_qp] * _test[_i][_qp];
}
