//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureCylindricalBase.h"

InputParameters
HeatStructureCylindricalBase::validParams()
{
  InputParameters params = HeatStructureBase::validParams();
  return params;
}

HeatStructureCylindricalBase::HeatStructureCylindricalBase(const InputParameters & params)
  : HeatStructureBase(params)
{
}

void
HeatStructureCylindricalBase::setupMesh()
{
  if (!_connected_to_flow_channel)
    _axial_offset = _inner_radius;

  HeatStructureBase::setupMesh();
}

void
HeatStructureCylindricalBase::addMooseObjects()
{
  HeatStructureBase::addMooseObjects();

  _hc_model->addHeatEquationRZ();
}

Real
HeatStructureCylindricalBase::getUnitPerimeter(const HeatStructureSideType & side) const
{
  switch (side)
  {
    case HeatStructureSideType::OUTER:
      return 2 * M_PI * (_inner_radius + _total_width);

    case HeatStructureSideType::INNER:
      return 2 * M_PI * _inner_radius;

    case HeatStructureSideType::START:
    case HeatStructureSideType::END:
      return std::numeric_limits<Real>::quiet_NaN();
  }

  mooseError(name(), ": Unknown value of 'side' parameter.");
}
