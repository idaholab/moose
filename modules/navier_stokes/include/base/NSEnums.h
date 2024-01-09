//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"

MooseEnum getPhaseEnum();
MooseEnum getLocalityEnum();
MooseEnum getSplittingEnum();

namespace NS
{
enum class ViscousForm : int
{
  Traction,
  Laplace
};

namespace phase
{
enum PhaseEnum
{
  fluid,
  solid
};
}

namespace settings
{
enum LocalityEnum
{
  local,
  global
};
}

namespace splitting
{
enum SplittingEnum
{
  porosity,
  thermal_conductivity,
  effective_thermal_conductivity
};
}
}
