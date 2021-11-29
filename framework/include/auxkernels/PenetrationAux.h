//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class PenetrationLocator;

class PenetrationAux : public AuxKernel
{
public:
  static InputParameters validParams();

  PenetrationAux(const InputParameters & parameters);

protected:
  enum PA_ENUM
  {
    PA_DISTANCE,
    PA_TANG_DISTANCE,
    PA_NORMAL_X,
    PA_NORMAL_Y,
    PA_NORMAL_Z,
    PA_CLOSEST_POINT_X,
    PA_CLOSEST_POINT_Y,
    PA_CLOSEST_POINT_Z,
    PA_ELEM_ID,
    PA_SIDE,
    PA_INCREMENTAL_SLIP_MAG,
    PA_INCREMENTAL_SLIP_X,
    PA_INCREMENTAL_SLIP_Y,
    PA_INCREMENTAL_SLIP_Z,
    PA_ACCUMULATED_SLIP,
    PA_FORCE_X,
    PA_FORCE_Y,
    PA_FORCE_Z,
    PA_NORMAL_FORCE_MAG,
    PA_NORMAL_FORCE_X,
    PA_NORMAL_FORCE_Y,
    PA_NORMAL_FORCE_Z,
    PA_TANGENTIAL_FORCE_MAG,
    PA_TANGENTIAL_FORCE_X,
    PA_TANGENTIAL_FORCE_Y,
    PA_TANGENTIAL_FORCE_Z,
    PA_FRICTIONAL_ENERGY,
    PA_LAGRANGE_MULTIPLIER,
    PA_MECH_STATUS
  };

  PA_ENUM _quantity;

  virtual Real computeValue() override;

  PenetrationLocator & _penetration_locator;

  const bool _has_secondary_gap_offset;
  MooseVariable * _secondary_gap_offset_var;

  const bool _has_mapped_primary_gap_offset;
  MooseVariable * _mapped_primary_gap_offset_var;

public:
  static const Real NotPenetrated;
};
