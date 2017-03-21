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

#include "FluxAverageAux.h"

template <>
InputParameters
validParams<FluxAverageAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("coupled", "Coupled variable for calculation of the flux");
  params.addRequiredParam<Real>("diffusivity", "Value to use as the 'diffusivity'");

  return params;
}

FluxAverageAux::FluxAverageAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _diffusivity(getParam<Real>("diffusivity")),
    _coupled_gradient(coupledGradient("coupled")),
    _coupled_var(*(getCoupledVars().find("coupled")->second)[0]),
    _normals(_coupled_var.normals())
{
}

Real
FluxAverageAux::computeValue()
{
  return _diffusivity * _coupled_gradient[_qp] * _normals[_qp];
}
