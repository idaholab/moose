//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSEnums.h"

MooseEnum
getPhaseEnum()
{
  return MooseEnum("fluid solid");
}

MooseEnum
getLocalityEnum()
{
  return MooseEnum("local global");
}

MooseEnum
getSplittingEnum()
{
  return MooseEnum("porosity thermal_conductivity effective_thermal_conductivity");
}
