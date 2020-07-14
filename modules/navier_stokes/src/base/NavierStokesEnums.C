//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesEnums.h"

MooseEnum getRadiationEnum()
{
  return MooseEnum("BreitbachBarthels KuniiSmith SinghKaviany Vortmeyer "
    "WakaoKato ZBS None Tsotsas", "BreitbachBarthels");
}

MooseEnum getFluidConductionEnum()
{
  return MooseEnum("DeisslerEian Hsu2D Hsu3D KuniiSmith Krupiczka ZBS None", "ZBS");
}

MooseEnum getSolidConductionEnum()
{
  return MooseEnum("ChanTien Hsu Hsu2D Hsu3D ZBS None", "ZBS");
}

MooseEnum getPhaseEnum()
{
  return MooseEnum("fluid solid");
}

MooseEnum getFluidEnum()
{
  return MooseEnum("air nitrogen helium");
}

MooseEnum getOnOffEnum()
{
  return MooseEnum("none standard", "standard");
}

MooseEnum getLocalityEnum()
{
  return MooseEnum("local global");
}

MooseEnum getSplittingEnum()
{
  return MooseEnum("porosity thermal_conductivity effective_thermal_conductivity");
}

MooseEnum getGeometryEnum()
{
  return MooseEnum("cylindrical cartesian", "cartesian");
}

MooseEnum getProfileEnum()
{
  return MooseEnum("axial radial", "radial");
}

MooseEnum getCoordinationNumberEnum()
{
  return MooseEnum("duToit Manegold Meissner Nakagaki Yang You", "Meissner");
}

MooseEnum getInterpolationEnum()
{
  return MooseEnum("interpolation nearest", "interpolation");
}

MooseEnum getMixingEnum()
{
  return MooseEnum("series parallel maxwell nielsen chiew", "series");
}

MooseEnum getSampleEnum()
{
  return MooseEnum("direct interpolate average", "direct");
}

MooseEnum getNearWallEnum()
{
  return MooseEnum("scaling correlation none", "none");
}

MooseEnum getVariablesEnum()
{
  return MooseEnum("conservative mixed primitive mixed_superficial primitive_superficial");
}

MooseEnum getModelEnum()
{
  return MooseEnum("euler navier_stokes friction_dominated");
}

MooseEnum getEntranceEnum()
{
  return MooseEnum("bottom top outer_radius inner_radius none", "none");
}
