//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUM_H
#define INSADMOMENTUM_H

#include "ADKernelValue.h"
#include "ADKernelGrad.h"
#include "ADKernelSUPG.h"

/****************************************************************/
/**************** INSADMomentumAdvection ************************/
/****************************************************************/

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
class INSADMomentumAdvection : public ADKernelValue<compute_stage>
{
public:
  INSADMomentumAdvection(const InputParameters & parameters);

protected:
  virtual ADResidual precomputeQpResidual() override;

  usingKernelValueMembers;
};

/****************************************************************/
/**************** INSADMomentumViscous **************************/
/****************************************************************/

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
  virtual ADVectorGradResidual precomputeQpResidual() override;

  usingVectorKernelGradMembers;
};

/****************************************************************/
/**************** INSADMomentumSUPG *****************************/
/****************************************************************/

// Forward Declarations
template <ComputeStage>
class INSADMomentumSUPG;

declareADValidParams(INSADMomentumSUPG);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the convective term of the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumSUPG : public ADVectorKernelSUPG<compute_stage>
{
public:
  INSADMomentumSUPG(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpStrongResidual() override;

  usingVectorKernelSUPGMembers;
};

#endif // INSADMOMENTUM_H
