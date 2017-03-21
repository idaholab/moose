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

#include "LateDeclarationVectorPostprocessor.h"

template <>
InputParameters
validParams<LateDeclarationVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<VectorPostprocessorValue>("value",
                                                    "The vector value this object will have.");
  return params;
}

LateDeclarationVectorPostprocessor::LateDeclarationVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _value(nullptr)
{
}

void
LateDeclarationVectorPostprocessor::initialize()
{
  if (_t_step == 1)
    _value = &declareVector("value");
}

void
LateDeclarationVectorPostprocessor::execute()
{
  if (_t_step == 1)
    *_value = getParam<VectorPostprocessorValue>("value");
}
