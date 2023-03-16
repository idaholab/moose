//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXBoundaryValueUserObject.h"

registerMooseObject("FunctionalExpansionToolsApp", FXBoundaryValueUserObject);

InputParameters
FXBoundaryValueUserObject::validParams()
{
  InputParameters params = FXBoundaryBaseUserObject::validParams();

  params.addClassDescription("Generates an Functional Expansion representation for a boundary "
                             "value condition using a 'FunctionSeries'-type Function");

  return params;
}

FXBoundaryValueUserObject::FXBoundaryValueUserObject(const InputParameters & parameters)
  : FXBoundaryBaseUserObject(parameters)
{
}
