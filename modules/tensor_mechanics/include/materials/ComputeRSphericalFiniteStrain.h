/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERSPHERICALFINITESTRAIN_H
#define COMPUTERSPHERICALFINITESTRAIN_H

#include "ComputeFiniteStrain.h"

/**
 * ComputeRSphericalFiniteStrain defines a strain increment and a rotation increment
 * for finite strains in 1D spherical symmetry geometries.  The strains in the
 * polar and azimuthal directions are functions of the radial displacement.

 */
class ComputeRSphericalFiniteStrain : public ComputeFiniteStrain
{
public:
  ComputeRSphericalFiniteStrain(const InputParameters & parameters);

protected:
  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  virtual void computeProperties();
};

#endif //COMPUTERSPHERICALFINITESTRAIN_H
