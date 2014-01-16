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

#include "DiffusionFluxBC.h"

template<>
InputParameters validParams<DiffusionFluxBC>()
{
  InputParameters params = validParams<FluxBC>();

  return params;
}

DiffusionFluxBC::DiffusionFluxBC(const std::string & name, InputParameters parameters) :
    FluxBC(name, parameters)
{
}

DiffusionFluxBC::~DiffusionFluxBC()
{
}

RealGradient
DiffusionFluxBC::computeQpFluxResidual()
{
  return _grad_u[_qp];
}

RealGradient
DiffusionFluxBC::computeQpFluxJacobian()
{
  return _grad_phi[_j][_qp];
}
