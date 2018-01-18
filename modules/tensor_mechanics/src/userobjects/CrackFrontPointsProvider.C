/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrackFrontPointsProvider.h"

template <>
InputParameters
validParams<CrackFrontPointsProvider>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Base class for a class that can provide a set of points along a "
                             "crack front. The virtual functions must be overridden by a derived "
                             "class to provide this functionality.");
  return params;
}

CrackFrontPointsProvider::CrackFrontPointsProvider(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
}
