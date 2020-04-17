//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADStressDivergenceTensors.h"

/**
 * ADStressDivergenceRZTensors is the automatic differentiation version of
 * StressDivergenceRZTensors. Within this kernel the first component of the displacements refers to
 * displacement in the radial direction and the second component refers to displacement in the
 * axial direction. The COORD_TYPE in the Problem block must be set to RZ.
 */
class ADStressDivergenceRZTensors : public ADStressDivergenceTensors
{
public:
  static InputParameters validParams();

  ADStressDivergenceRZTensors(const InputParameters & parameters);

protected:
  void initialSetup() override;

  ADReal computeQpResidual() override;
  void precalculateResidual() override;
};
