/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*   Pronghorn: Coarse-Mesh, Multi-Dimensional, Thermal-Hydraulics  */
/*                                                                  */
/*              (c) 2020 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "MooseEnum.h"

MooseEnum getPhaseEnum();
MooseEnum getLocalityEnum();
MooseEnum getSplittingEnum();

namespace NS
{
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
