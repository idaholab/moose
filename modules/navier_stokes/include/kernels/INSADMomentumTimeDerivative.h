//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUMTIMEDERIVATIVE_H
#define INSADMOMENTUMTIMEDERIVATIVE_H

#include "ADTimeKernel.h"

// Forward Declarations
template <ComputeStage compute_stage>
class INSADMomentumTimeDerivative;

declareADValidParams(INSADMomentumTimeDerivative);

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation.  Could instead use CoefTimeDerivative
 * for this.
 */
template <ComputeStage compute_stage>
class INSADMomentumTimeDerivative : public ADVectorTimeKernel<compute_stage>
{
public:
  INSADMomentumTimeDerivative(const InputParameters & parameters);

  virtual ~INSADMomentumTimeDerivative() {}

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty(Real) & _rho;

  usingVectorTimeKernelMembers;
};

#endif // INSADMOMENTUMTIMEDERIVATIVE_H
