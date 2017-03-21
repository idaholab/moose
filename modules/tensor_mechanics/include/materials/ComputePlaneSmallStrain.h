/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPLANESMALLSTRAIN_H
#define COMPUTEPLANESMALLSTRAIN_H

#include "Compute2DSmallStrain.h"

/**
 * ComputePlaneSmallStrain defines small strains under generalized
 * plane strain and plane stress assumptions, where the out of plane strain
 * can be uniformly or non-uniformly zero or nonzero.
 */
class ComputePlaneSmallStrain : public Compute2DSmallStrain
{
public:
  ComputePlaneSmallStrain(const InputParameters & parameters);

protected:
  virtual Real computeStrainZZ();

private:
  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;

  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
};

#endif // COMPUTEPLANESMALLSTRAIN_H
