//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCurvatureAux.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", ADCurvatureAux);

InputParameters
ADCurvatureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  return params;
}

ADCurvatureAux::ADCurvatureAux(const InputParameters & parameters)
  : AuxKernel(parameters), _curvatures(_assembly.adCurvatures())
{
  if (!_bnd)
    mooseError("This class can only be run on a boundary");

  if (isNodal())
    paramError(
        "variable",
        "This class only supports elemental variables as curvatures are not defined on nodes.");

  _subproblem.haveADObjects(true);
}

Real
ADCurvatureAux::computeValue()
{
  return MetaPhysicL::raw_value(_curvatures[_qp]);
}
