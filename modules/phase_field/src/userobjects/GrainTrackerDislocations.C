//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTrackerDislocations.h"

registerMooseObject("PhaseFieldApp", GrainTrackerDislocations);

InputParameters
GrainTrackerDislocations::validParams()
{
  InputParameters params = GrainTracker::validParams();

  params.addClassDescription(
      "GrainTracker extension with grain dislocation density and formation time");

  params.addRequiredParam<UserObjectName>(
      "dislocation_density_reader",
      "Name of grain-dependent intial dislocation density user object");
  params.addParam<bool>("add_default_density_grains",
                        false,
                        "flag to generate grains with default dislocation density");

  params.addParam<Real>("default_density", 0.0, "default dislocation density for new grains");

  return params;
}

GrainTrackerDislocations::GrainTrackerDislocations(const InputParameters & parameters)
  : GrainDataTracker<Real>(parameters),
    _default_density_grains(getParam<bool>("add_default_density_grains")),
    _default_density(getParam<Real>("default_density")),
    _density(getUserObject<DislocationDensityFileReader>("dislocation_density_reader"))
{
}

void
GrainTrackerDislocations::newGrainCreated(unsigned int new_grain_id)
{
  if (_grain_data.size() <= new_grain_id)
    _grain_data.resize(new_grain_id + 1);

  _grain_data[new_grain_id] = newGrain(new_grain_id);

  if (_formation_data.size() <= new_grain_id)
    _formation_data.resize(new_grain_id + 1);

  _formation_data[new_grain_id] = _t;
}

Real
GrainTrackerDislocations::newGrain(unsigned int new_grain_id)
{
  Real grain_density;

  if (new_grain_id < _density.getGrainNum())
    grain_density = _density.getDensity(new_grain_id);
  else
  {
    if (_default_density_grains)
      grain_density = _default_density;

    else
      mooseError("GrainTrackerDislocations has run out of dislocation densities for grains");
  }

  return grain_density;
}

Real
GrainTrackerDislocations::getFormationTime(unsigned int grain_id) const
{
  return _formation_data[grain_id];
}
