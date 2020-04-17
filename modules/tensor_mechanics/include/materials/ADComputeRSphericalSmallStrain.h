//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeSmallStrain.h"

/**
 * ADComputeRSphericalSmallStrain defines a strain tensor, assuming small strains,
 * in a 1D simulation assumming spherical symmetry.  The polar and azimuthal
 * strains are functions of the radial displacement and radial position in this
 * 1D problem.
 */
class ADComputeRSphericalSmallStrain : public ADComputeSmallStrain
{
public:
  static InputParameters validParams();

  ADComputeRSphericalSmallStrain(const InputParameters & parameters);

  virtual void computeProperties() override;
};
