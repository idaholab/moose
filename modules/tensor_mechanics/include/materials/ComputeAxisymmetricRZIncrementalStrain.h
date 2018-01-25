//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEAXISYMMETRICRZINCREMENTALSTRAIN_H
#define COMPUTEAXISYMMETRICRZINCREMENTALSTRAIN_H

#include "Compute2DIncrementalStrain.h"

class ComputeAxisymmetricRZIncrementalStrain;

template <>
InputParameters validParams<ComputeAxisymmetricRZIncrementalStrain>();

/**
 * ComputeAxisymmetricRZIncrementalStrain defines a strain increment only
 * for incremental strains in an Axisymmetric simulation.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetricRZIncrementalStrain : public Compute2DIncrementalStrain
{
public:
  ComputeAxisymmetricRZIncrementalStrain(const InputParameters & parameters);

protected:
  void initialSetup() override;

  /// Computes the current dUz/dz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  Real computeGradDispZZ() override;

  /// Computes the old dUz/dz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  Real computeGradDispZZOld() override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};

#endif // COMPUTEAXISYMMETRICRZINCREMENTALSTRAIN_H
