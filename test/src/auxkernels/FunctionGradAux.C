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

#include "FunctionGradAux.h"
#include "Function.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<FunctionGradAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<FunctionName>("function", "Function used to compute gradient");
  params.addRequiredParam<unsigned int>("dimension_index", "The dimension x=0|y=1|z=2");
  return params;
}

FunctionGradAux::FunctionGradAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _func(getFunction("function")),
    _dim_index(getParam<unsigned int>("dimension_index"))
{
  if (_dim_index > _mesh.dimension())
    mooseError("dimension_index > mesh dimension");
}

FunctionGradAux::~FunctionGradAux()
{
}

Real
FunctionGradAux::computeValue()
{
  return _func.gradient(_t, _q_point[_qp])(_dim_index);
}
