//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FusionProblem.h"

registerMooseObject("MooseApp", FusionProblem);

InputParameters
FusionProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addParam<BoundaryName>("master_bdry_name",
                                "Boundary name in master subapp wants to transfer data from/to. ");

  params.addClassDescription("A Problem object that customized setup for fusion simulations");

  return params;
}

FusionProblem::FusionProblem(const InputParameters & parameters)
  : FEProblem(parameters),
    _master_bdry_name(isParamValid("master_bdry_name") ? getParam<BoundaryName>("master_bdry_name")
                                                         : "")
{
}
