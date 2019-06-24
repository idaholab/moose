//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeSmallStrainNOSPD.h"

class ComputeForceStabilizedSmallStrainNOSPD;

template <>
InputParameters validParams<ComputeForceStabilizedSmallStrainNOSPD>();

/**
 * Material class for conventional correspondence material model with fictitious force stabilization
 * for small strain
 */
class ComputeForceStabilizedSmallStrainNOSPD : public ComputeSmallStrainNOSPD
{
public:
  ComputeForceStabilizedSmallStrainNOSPD(const InputParameters & parameters);

protected:
  virtual void computeQpDeformationGradient() override;

  /// Elasticity tensor
  const MaterialProperty<RankFourTensor> & _Cijkl;

  /// Material property for fictitious force
  MaterialProperty<Real> & _sf_coeff;
};
