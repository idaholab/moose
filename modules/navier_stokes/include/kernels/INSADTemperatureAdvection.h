//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * This class computes the residual and Jacobian contributions for
 * temperature advection
 */
template <ComputeStage compute_stage>
class INSADTemperatureAdvection : public ADKernelValue<compute_stage>
{
public:
  static InputParameters validParams();

  INSADTemperatureAdvection(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADMaterialProperty(Real) & _rho;
  const ADMaterialProperty(Real) & _cp;
  const ADVectorVariableValue & _U;

  usingKernelValueMembers;
};

#include "ADKernelSUPG.h"

// Forward Declarations
template <ComputeStage>
class INSADTemperatureAdvectionSUPG;

/**
 * This class computes the residual and Jacobian contributions for
 * stabilization of temperature advection
 */
template <ComputeStage compute_stage>
class INSADTemperatureAdvectionSUPG : public ADKernelSUPG<compute_stage>
{
public:
  static InputParameters validParams();

  INSADTemperatureAdvectionSUPG(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpStrongResidual() override;

  const ADMaterialProperty(Real) & _rho;
  const ADMaterialProperty(Real) & _cp;
  const ADVectorVariableValue & _U;

  usingKernelSUPGMembers;
};
