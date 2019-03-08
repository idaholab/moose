//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUMADVECTION_H
#define INSADMOMENTUMADVECTION_H

#include "ADKernelValue.h"

// Forward Declarations
template <ComputeStage>
class INSADMomentumAdvection;

declareADValidParams(INSADMomentumAdvection);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the convective term of the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumAdvection : public ADVectorKernelValue<compute_stage>
{
public:
  INSADMomentumAdvection(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpResidual() override;

  const ADMaterialProperty(RealVectorValue) & _convective_strong_residual;

  usingVectorKernelValueMembers;
};

#endif // INSADMOMENTUMADVECTION_H
