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

#include "VectorNeumannBC.h"

template<>
InputParameters validParams<VectorNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<RealVectorValue>("vector_value", RealVectorValue(), "vector this BC should act in");
  return params;
}

VectorNeumannBC::VectorNeumannBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _value(getParam<RealVectorValue>("vector_value"))
{}

Real
VectorNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp]*(_value*_normals[_qp]);
}

