/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEAXISYMMETRICRZINCREMENTALPLANESTRAIN_H
#define COMPUTEAXISYMMETRICRZINCREMENTALPLANESTRAIN_H

#include "Compute1DIncrementalStrain.h"

/**
 * ComputeAxisymmetricRZIncrementalPlaneStrain defines a strain increment only
 * for incremental strains in an Axisymmetric simulation with plane strain in
 * the y direction.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZIncrementalPlaneStrain : public Compute1DIncrementalStrain
{
public:
  ComputeAxisymmetricRZIncrementalPlaneStrain(const InputParameters & parameters);

protected:
  virtual void initialSetup();

  /// Computes the current deformation gradient in the y direction
  virtual Real computeDUYDY();

  /// Computes the old deformation gradient in the y direction
  virtual Real computeDUYDYOld();

  /// Computes the current deformation gradient for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  virtual Real computeDUZDZ();

  /// Computes the old  deformation gradient for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  virtual Real computeDUZDZOld();

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  bool _have_strain_yy;
  const VariableValue & _strain_yy;
  const VariableValue & _strain_yy_old;
  bool _have_scalar_strain_yy;
  const VariableValue & _scalar_strain_yy;
  const VariableValue & _scalar_strain_yy_old;
};

#endif //COMPUTEAXISYMMETRICRZINCREMENTALPLANESTRAIN_H
