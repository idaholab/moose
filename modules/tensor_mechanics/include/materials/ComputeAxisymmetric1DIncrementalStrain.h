/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEAXISYMMETRIC1DINCREMENTALSTRAIN_H
#define COMPUTEAXISYMMETRIC1DINCREMENTALSTRAIN_H

#include "Compute1DIncrementalStrain.h"

/**
 * ComputeAxisymmetric1DIncrementalStrain defines a strain increment only
 * for incremental small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetric1DIncrementalStrain : public Compute1DIncrementalStrain
{
public:
  ComputeAxisymmetric1DIncrementalStrain(const InputParameters & parameters);

protected:
  void initialSetup() override;

  /// Computes the current dUy/dy for axisymmetric problems
  Real computeGradDispYY() override;

  /// Computes the old dUy/dy for axisymmetric problems
  Real computeGradDispYYOld() override;

  /// Computes the current dUz/dz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  Real computeGradDispZZ() override;

  /// Computes the old dUz/dz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  Real computeGradDispZZOld() override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;

  bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  const VariableValue & _scalar_out_of_plane_strain_old;
};

#endif // COMPUTEAXISYMMETRIC1DINCREMENTALSTRAIN_H
