//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUMVISCOUS_H
#define INSADMOMENTUMVISCOUS_H

#include "ADKernelGrad.h"

// Forward Declarations
template <ComputeStage>
class INSADMomentumViscous;

declareADValidParams(INSADMomentumViscous);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the viscous term of the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumViscous : public ADVectorKernelGrad<compute_stage>
{
public:
  INSADMomentumViscous(const InputParameters & parameters);

protected:
  virtual ADTensorResidual precomputeQpResidual() override;

  const ADMaterialProperty(Real) & _mu;

  usingVectorKernelGradMembers;
};

#endif // INSADMOMENTUMVISCOUS_H
