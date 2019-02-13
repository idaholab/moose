//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTERSPHERICALSMALLSTRAIN_H
#define ADCOMPUTERSPHERICALSMALLSTRAIN_H

#include "ADComputeSmallStrain.h"

template <ComputeStage>
class ADComputeRSphericalSmallStrain;

declareADValidParams(ADComputeRSphericalSmallStrain);

/**
 * ADComputeRSphericalSmallStrain defines a strain tensor, assuming small strains,
 * in a 1D simulation assumming spherical symmetry.  The polar and azimuthal
 * strains are functions of the radial displacement and radial position in this
 * 1D problem.
 */
template <ComputeStage compute_stage>
class ADComputeRSphericalSmallStrain : public ADComputeSmallStrain<compute_stage>
{
public:
  ADComputeRSphericalSmallStrain(const InputParameters & parameters);

  virtual void computeProperties() override;

  usingComputeSmallStrainMembers;
};

#endif // ADCOMPUTERSPHERICALSMALLSTRAIN_H
