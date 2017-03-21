/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERSPHERICALSMALLSTRAIN_H
#define COMPUTERSPHERICALSMALLSTRAIN_H

#include "ComputeSmallStrain.h"

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
