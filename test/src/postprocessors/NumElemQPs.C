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

#include "NumElemQPs.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<NumElemQPs>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  return params;
}

NumElemQPs::NumElemQPs(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters)
{
}

NumElemQPs::~NumElemQPs() {}

Real
NumElemQPs::computeIntegral()
{
  return _qrule->n_points();
}

Real
NumElemQPs::computeQpIntegral()
{
  mooseError("Unimplemented method");
}
