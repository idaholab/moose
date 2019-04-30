//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADPresetNodalBC.h"

template <ComputeStage>
class ADPresetBC;

declareADValidParams(ADPresetBC);

/**
 * Defines a boundary condition that (pre)sets the solution at the boundary
 * to be a user specified value.
 */
template <ComputeStage compute_stage>
class ADPresetBC : public ADPresetNodalBC<compute_stage>
{
public:
  ADPresetBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpValue() override;

  const Real & _value;

  usingPresetNodalBCMembers;
};
