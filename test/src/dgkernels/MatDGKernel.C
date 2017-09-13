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

#include "MatDGKernel.h"

// MOOSE includes
#include "MooseVariable.h"

template <>
InputParameters
validParams<MatDGKernel>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "This is being tested.");
  return params;
}

MatDGKernel::MatDGKernel(const InputParameters & parameters)
  : DGKernel(parameters), _value(getMaterialProperty<Real>("mat_prop"))
{
}
