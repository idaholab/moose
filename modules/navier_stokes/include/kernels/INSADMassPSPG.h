//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

// Forward Declarations
template <ComputeStage>
class INSADMassPSPG;

declareADValidParams(INSADMassPSPG);

/**
 * This class adds PSPG stabilization to the mass equation, enabling use of
 * equal order shape functions for pressure and velocity variables
 */
template <ComputeStage compute_stage>
class INSADMassPSPG : public ADKernelGrad<compute_stage>
{
public:
  INSADMassPSPG(const InputParameters & parameters);

protected:
  ADRealVectorValue precomputeQpResidual() override;

  /// The density
  const ADMaterialProperty(Real) & _rho;
  /// The stabilization parameter tau
  const ADMaterialProperty(Real) & _tau;
  /// The strong residual of the momentum equation, computed using INSADMaterial
  const ADMaterialProperty(RealVectorValue) & _momentum_strong_residual;

  usingKernelGradMembers;
};

