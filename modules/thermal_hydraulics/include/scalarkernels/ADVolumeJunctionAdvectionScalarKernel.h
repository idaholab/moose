//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"

class ADVolumeJunctionBaseUserObject;

/**
 * Adds advective fluxes for the junction variables for a volume junction
 */
class ADVolumeJunctionAdvectionScalarKernel : public ADScalarKernel
{
public:
  ADVolumeJunctionAdvectionScalarKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Index within local system of the equation upon which this object acts
  const unsigned int _equation_index;

  /// Volume junction user object
  const ADVolumeJunctionBaseUserObject & _volume_junction_uo;

public:
  static InputParameters validParams();
};
