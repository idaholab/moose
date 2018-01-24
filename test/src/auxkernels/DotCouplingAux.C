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

#include "DotCouplingAux.h"

template <>
InputParameters
validParams<DotCouplingAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("v", "Coupled variable");

  return params;
}

DotCouplingAux::DotCouplingAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v_dot(coupledDot("v"))
{
}

DotCouplingAux::~DotCouplingAux() {}

Real
DotCouplingAux::computeValue()
{
  return _v_dot[_qp];
}
