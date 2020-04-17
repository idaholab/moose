//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCompute2DFiniteStrain.h"

/**
 * ADComputeAxisymmetricRZFiniteStrain defines a strain increment and rotation
 * increment for finite strains in an Axisymmetric simulation.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ADComputeAxisymmetricRZFiniteStrain : public ADCompute2DFiniteStrain
{
public:
  static InputParameters validParams();

  ADComputeAxisymmetricRZFiniteStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  ADReal computeOutOfPlaneGradDisp() override;

  Real computeOutOfPlaneGradDispOld() override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};
