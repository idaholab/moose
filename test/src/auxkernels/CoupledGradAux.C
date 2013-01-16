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

#include "CoupledGradAux.h"

template<>
InputParameters validParams<CoupledGradAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("coupled", "Coupled gradient for calculation");

  params.addRequiredParam<RealGradient>("grad", "Gradient to dot it with");

  return params;
}

CoupledGradAux::CoupledGradAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _grad(getParam<RealGradient>("grad")),
    _coupled(coupled("coupled")),
    _coupled_grad(coupledGradient("coupled"))
{
}

CoupledGradAux::~CoupledGradAux()
{
}

Real
CoupledGradAux::computeValue()
{
  return _coupled_grad[_qp]*_grad;
}
