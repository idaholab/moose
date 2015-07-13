/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEAXISYMMETRICRZSMALLSTRAIN_H
#define COMPUTEAXISYMMETRICRZSMALLSTRAIN_H

#include "Compute2DSmallStrain.h"

/**
 * ComputeAxisymmetricRZSmallStrain defines small strains in an Axisymmetric system.
 * Within this material, '_disp_x' refers to displacement in the radial direction,
 * u_r, and '_disp_y' refers to displacement in the axial direction, u_z.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZSmallStrain : public Compute2DSmallStrain
{
public:
  ComputeAxisymmetricRZSmallStrain(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeStrainZZ();
};

#endif //COMPUTEAXISYMMETRICRZSMALLSTRAIN_H
