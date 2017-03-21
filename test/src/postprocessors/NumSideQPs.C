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

#include "NumSideQPs.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<NumSideQPs>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  return params;
}

NumSideQPs::NumSideQPs(const InputParameters & parameters) : SideIntegralPostprocessor(parameters)
{
}

NumSideQPs::~NumSideQPs() {}

Real
NumSideQPs::computeIntegral()
{
  return _qrule->n_points();
}

Real
NumSideQPs::computeQpIntegral()
{
  mooseError("Unimplemented method");
}
