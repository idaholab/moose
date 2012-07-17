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

#include "FunctionPresetBC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionPresetBC>()
{
  InputParameters params = validParams<PresetNodalBC>();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  return params;
}

FunctionPresetBC::FunctionPresetBC(const std::string & name, InputParameters parameters) :
    PresetNodalBC(name, parameters),
    _func(getFunction("function"))
{
}

Real
FunctionPresetBC::computeQpValue()
{
  return _func.value(_t, *_current_node);
}
