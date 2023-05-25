//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontPointsProvider.h"

InputParameters
CrackFrontPointsProvider::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Base class for a class that can provide a set of points along a "
                             "crack front. The virtual functions must be overridden by a derived "
                             "class to provide this functionality.");
  return params;
}

CrackFrontPointsProvider::CrackFrontPointsProvider(const InputParameters & parameters,
                                                   const bool uses_mesh)
  : ElementUserObject(parameters), _uses_mesh(uses_mesh)
{
}
