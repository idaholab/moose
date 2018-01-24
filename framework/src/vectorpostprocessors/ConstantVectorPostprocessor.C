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

#include "ConstantVectorPostprocessor.h"

template <>
InputParameters
validParams<ConstantVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<VectorPostprocessorValue>("value",
                                                    "The vector value this object will have.");
  return params;
}

ConstantVectorPostprocessor::ConstantVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _value(declareVector("value"))
{
  _value = getParam<VectorPostprocessorValue>("value");
}

void
ConstantVectorPostprocessor::initialize()
{
}

void
ConstantVectorPostprocessor::execute()
{
}
