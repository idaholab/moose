//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUMSUPG_H
#define INSADMOMENTUMSUPG_H

#include "ADKernelSUPG.h"

// Forward Declarations
template <ComputeStage>
class INSADMomentumSUPG;

declareADValidParams(INSADMomentumSUPG);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for SUPG stabilization terms of the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumSUPG : public ADVectorKernelSUPG<compute_stage>
{
public:
  INSADMomentumSUPG(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpStrongResidual() override;

  const ADMaterialProperty(RealVectorValue) & _momentum_strong_residual;

  usingVectorKernelSUPGMembers;
};

#endif // INSADMOMENTUMSUPG_H
