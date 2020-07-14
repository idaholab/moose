#pragma once

#include "MooseEnum.h"

MooseEnum getRadiationEnum();
MooseEnum getFluidConductionEnum();
MooseEnum getSolidConductionEnum();
MooseEnum getPhaseEnum();
MooseEnum getFluidEnum();
MooseEnum getOnOffEnum();
MooseEnum getLocalityEnum();
MooseEnum getSplittingEnum();
MooseEnum getGeometryEnum();
MooseEnum getProfileEnum();
MooseEnum getCoordinationNumberEnum();
MooseEnum getInterpolationEnum();
MooseEnum getMixingEnum();
MooseEnum getSampleEnum();
MooseEnum getNearWallEnum();
MooseEnum getVariablesEnum();
MooseEnum getModelEnum();
MooseEnum getEntranceEnum();

namespace radiation
{
  enum RadiationEnum
  {
    BreitbachBarthels,
    KuniiSmith,
    SinghKaviany,
    Vortmeyer,
    WakaoKato,
    ZBS,
    None,
    Tsotsas
  };
}

namespace fluid_conduction
{
  enum FluidConductionEnum
  {
    DeisslerEian,
    Hsu2D,
    Hsu3D,
    KuniiSmith,
    Krupiczka,
    ZBS,
    None
  };
}

namespace solid_conduction
{
  enum SolidConductionEnum
  {
    ChanTien,
    Hsu,
    Hsu2D,
    Hsu3D,
    ZBS,
    None
  };
}

namespace phase
{
  enum PhaseEnum
  {
    fluid,
    solid
  };
}

namespace material
{
  enum FluidEnum
  {
    air,
    nitrogen,
    helium
  };
}

namespace wall
{
  enum NearWallEnum
  {
    scaling,
    correlation,
    none
  };
}

namespace settings
{
  enum OnOffEnum
  {
    none,
    standard
  };

  enum LocalityEnum
  {
    local,
    global
  };

  enum GeometryEnum
  {
    cylindrical,
    cartesian
  };

  enum ProfileEnum
  {
    axial,
    radial
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

namespace method
{
  enum CoordinationNumberEnum
  {
    duToit,
    Manegold,
    Meissner,
    Nakagaki,
    Yang,
    You
  };

  enum InterpolationEnum
  {
    interpolation,
    nearest
  };

  enum MixingEnum
  {
    series,
    parallel,
    maxwell,
    nielsen,
    chiew
  };

  enum SampleEnum
  {
    direct,
    interpolate,
    average
  };
}

namespace variables
{
  enum VariablesEnum
  {
    conservative,
    mixed,
    primitive,
    mixed_superficial,
    primitive_superficial
  };
}

namespace model
{
  enum ModelEnum
  {
    euler,
    navier_stokes,
    friction_dominated
  };
}

namespace entrance
{
  enum EntranceEnum
  {
    bottom,
    top,
    outer_radius,
    inner_radius,
    none
  };
}
