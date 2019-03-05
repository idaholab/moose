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
class INSADMomentumAdvection : public ADVectorKernelValue<compute_stage>
{
public:
  INSADMomentumAdvection(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty(RealVectorValue) & _convective_strong_residual;

  usingVectorKernelValueMembers;
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

  const ADMaterialProperty(Real) & _mu;

  usingVectorKernelGradMembers;
};

/****************************************************************/
/**************** INSADMomentumPressure **************************/
/****************************************************************/

// Forward Declarations
template <ComputeStage>
class INSADMomentumPressure;

declareADValidParams(INSADMomentumPressure);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the pressure term of the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumPressure : public ADVectorKernel<compute_stage>
{
public:
  INSADMomentumPressure(const InputParameters & parameters);

protected:
  virtual ADVectorResidual computeQpResidual() override;

  const bool _integrate_p_by_parts;
  const ADVariableValue & _p;
  const ADVariableGradient & _grad_p;

  usingVectorKernelMembers;
};

/****************************************************************/
/**************** INSADMomentumForces **************************/
/****************************************************************/

// Forward Declarations
template <ComputeStage>
class INSADMomentumForces;

declareADValidParams(INSADMomentumForces);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for force terms in the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumForces : public ADVectorKernelValue<compute_stage>
{
public:
  INSADMomentumForces(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty(RealVectorValue) & _gravity_strong_residual;
  const ADMaterialProperty(RealVectorValue) & _mms_function_strong_residual;

  usingVectorKernelValueMembers;
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
 * contributions for SUPG stabilization terms of the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumSUPG : public ADVectorKernelSUPG<compute_stage>
{
public:
  INSADMomentumSUPG(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpStrongResidual() override;

  const ADMaterialProperty(RealVectorValue) & _momentum_strong_residual;

  usingVectorKernelSUPGMembers;
};

#endif // INSADMOMENTUM_H
