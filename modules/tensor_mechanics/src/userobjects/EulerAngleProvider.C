/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EulerAngleProvider.h"

template <>
InputParameters
validParams<EulerAngleProvider>()
{
  return validParams<GeneralUserObject>();
}
