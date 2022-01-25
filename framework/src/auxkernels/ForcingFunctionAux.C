//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ForcingFunctionAux.h"
#include "Function.h"

registerMooseObject("MooseApp", ForcingFunctionAux);

InputParameters
ForcingFunctionAux::validParams()
{
  InputParameters params = FunctionAux::validParams();
  params.addClassDescription("Auxiliary Kernel that adds a forcing function to the value of an "
                             "AuxVariable from the previous time step.");
  return params;
}

ForcingFunctionAux::ForcingFunctionAux(const InputParameters & parameters)
  : FunctionAux(parameters), _u_old(uOld())
{
  if (isNodal())
    paramError("variable", "The variable must be elemental");
}

Real
ForcingFunctionAux::computeValue()
{
  return _u_old[_qp] + _dt * _func.value(_t, _q_point[_qp]);
}
