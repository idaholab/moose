//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTERSPHERICALSMALLSTRAIN_H
#define COMPUTERSPHERICALSMALLSTRAIN_H

#include "ComputeSmallStrain.h"

class ComputeRSphericalSmallStrain;

template <>
InputParameters validParams<ComputeRSphericalSmallStrain>();

/**
 * ComputeRSphericalSmallStrain defines a strain tensor, assuming small strains,
 * in a 1D simulation assumming spherical symmetry.  The polar and azimuthal
 * strains are functions of the radial displacement and radial position in this
 * 1D problem.
 */
class ComputeRSphericalSmallStrain : public ComputeSmallStrain
{
public:
  ComputeRSphericalSmallStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
};

#endif // COMPUTERSPHERICALSMALLSTRAIN_H
