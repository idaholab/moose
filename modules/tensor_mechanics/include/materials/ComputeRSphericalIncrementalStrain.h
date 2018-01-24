/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERSPHERICALINCREMENTALSTRAIN_H
#define COMPUTERSPHERICALINCREMENTALSTRAIN_H

#include "ComputeIncrementalSmallStrain.h"

/**
 * ComputeRSphericalIncrementalStrain defines a strain increment only
 * for small strains in 1D spherical symmetry geometries.  The strains in the
 * polar and azimuthal directions are functions of the radial displacement.

 */
class ComputeRSphericalIncrementalStrain : public ComputeIncrementalSmallStrain
{
public:
  ComputeRSphericalIncrementalStrain(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  /// Computes the current and old deformation gradients with the assumptions for
  /// 1D spherical symmetry geometries: \f$ \epsilon_{\theta} = \epsilon_{\phi} = \frac{u_r}{r} \f$
  virtual void computeTotalStrainIncrement(RankTwoTensor & total_strain_increment) override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};

#endif // COMPUTERSPHERICALINCREMENTALSTRAIN_H
