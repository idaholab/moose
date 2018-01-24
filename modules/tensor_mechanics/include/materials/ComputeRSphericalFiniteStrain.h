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
  virtual void initialSetup();

  /// Computes the current and old deformation gradients with the assumptions for
  /// 1D spherical symmetry geometries: \f$ \epsilon_{\theta} = \epsilon_{\phi} = \frac{u_r}{r} \f$
  virtual void computeProperties();

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};

#endif // COMPUTERSPHERICALFINITESTRAIN_H
