//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute2DIncrementalStrain.h"

/**
 * ComputeAxisymmetricRZIncrementalStrain defines a strain increment only
 * for incremental strains in an Axisymmetric simulation.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZIncrementalStrain : public Compute2DIncrementalStrain
{
public:
  static InputParameters validParams();

  ComputeAxisymmetricRZIncrementalStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  Real computeOutOfPlaneGradDisp() override;

  Real computeOutOfPlaneGradDispOld() override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};
