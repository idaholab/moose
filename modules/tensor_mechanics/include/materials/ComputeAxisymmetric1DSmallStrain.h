/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEAXISYMMETRIC1DSMALLSTRAIN_H
#define COMPUTEAXISYMMETRIC1DSMALLSTRAIN_H

#include "Compute1DSmallStrain.h"

/**
 * ComputeAxisymmetric1DSmallStrain defines small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetric1DSmallStrain : public Compute1DSmallStrain
{
public:
  ComputeAxisymmetric1DSmallStrain(const InputParameters & parameters);

protected:
  void initialSetup() override;

  /// Computes the strain_yy for axisymmetric problems
  Real computeStrainYY() override;

  /// Computes the strain_zz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  Real computeStrainZZ() override;

  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;

  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
};

#endif // COMPUTEAXISYMMETRIC1DSMALLSTRAIN_H
