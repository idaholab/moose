//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEAXISYMMETRICRZFINITESTRAIN_H
#define COMPUTEAXISYMMETRICRZFINITESTRAIN_H

#include "Compute2DFiniteStrain.h"

class ComputeAxisymmetricRZFiniteStrain;

template <>
InputParameters validParams<ComputeAxisymmetricRZFiniteStrain>();

/**
 * ComputeAxisymmetricRZFiniteStrain defines a strain increment and rotation
 * increment for finite strains in an Axisymmetric simulation.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZFiniteStrain : public Compute2DFiniteStrain
{
public:
  ComputeAxisymmetricRZFiniteStrain(const InputParameters & parameters);

protected:
  void initialSetup() override;

  /// Computes the current dUz/dz for axisymmetric problems, where
  /// \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  Real computeGradDispZZ() override;

  /// Computes the old dUz/dz for axisymmetric problems, where
  /// \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  Real computeGradDispZZOld() override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};

#endif // COMPUTEAXISYMMETRICRZFINITESTRAIN_H
