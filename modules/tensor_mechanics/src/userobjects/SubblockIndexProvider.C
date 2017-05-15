/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SubblockIndexProvider.h"

template <>
InputParameters
validParams<SubblockIndexProvider>()
{
  return validParams<GeneralUserObject>();
}
