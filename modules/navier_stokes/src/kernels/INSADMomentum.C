//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentum.h"

/****************************************************************/
/**************** INSADMomentumAdvection ************************/
/****************************************************************/

registerADMooseObject("NavierStokesApp", INSADMomentumAdvection);

defineADValidParams(
    INSADMomentumAdvection,
    ADVectorKernelValue,
    params.addClassDescription("Adds the convective term to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumAdvection<compute_stage>::INSADMomentumAdvection(const InputParameters & parameters)
  : ADVectorKernelValue<compute_stage>(parameters),
    _convective_strong_residual(
        adGetADMaterialProperty<RealVectorValue>("convective_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
INSADMomentumAdvection<compute_stage>::precomputeQpResidual()
{
  return _convective_strong_residual[_qp];
}

/****************************************************************/
/**************** INSADMomentumViscous **************************/
/****************************************************************/

registerADMooseObject("NavierStokesApp", INSADMomentumViscous);

defineADValidParams(
    INSADMomentumViscous,
    ADVectorKernelGrad,
    params.addClassDescription("Adds the viscous term to the INS momentum equation");
    params.addParam<MaterialPropertyName>("mu_name",
                                          "mu",
                                          "The name of the viscosity material property"););

template <ComputeStage compute_stage>
INSADMomentumViscous<compute_stage>::INSADMomentumViscous(const InputParameters & parameters)
  : ADVectorKernelGrad<compute_stage>(parameters), _mu(adGetADMaterialProperty<Real>("mu_name"))
{
}

template <ComputeStage compute_stage>
ADTensorResidual
INSADMomentumViscous<compute_stage>::precomputeQpResidual()
{
  return _mu[_qp] * _grad_u[_qp];
}

/****************************************************************/
/**************** INSADMomentumPressure *************************/
/****************************************************************/

registerADMooseObject("NavierStokesApp", INSADMomentumPressure);

defineADValidParams(
    INSADMomentumPressure,
    ADVectorKernel,
    params.addClassDescription("Adds the pressure term to the INS momentum equation");
    params.addRequiredCoupledVar("p", "The pressure");
    params.addParam<bool>("integrate_p_by_parts",
                          true,
                          "Whether to integrate the pressure term by parts"););

template <ComputeStage compute_stage>
INSADMomentumPressure<compute_stage>::INSADMomentumPressure(const InputParameters & parameters)
  : ADVectorKernel<compute_stage>(parameters),
    _integrate_p_by_parts(adGetParam<bool>("integrate_p_by_parts")),
    _p(adCoupledValue("p")),
    _grad_p(adCoupledGradient("p"))
{
}

template <ComputeStage compute_stage>
ADResidual
INSADMomentumPressure<compute_stage>::computeQpResidual()
{
  if (_integrate_p_by_parts)
    return -_p[_qp] * _grad_test[_i][_qp].tr();
  else
    return _test[_i][_qp] * _grad_p[_qp];
}

/****************************************************************/
/**************** INSADMomentumForces ***************************/
/****************************************************************/

registerADMooseObject("NavierStokesApp", INSADMomentumForces);

defineADValidParams(INSADMomentumForces,
                    ADVectorKernelValue,
                    params.addClassDescription("Adds body forces to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumForces<compute_stage>::INSADMomentumForces(const InputParameters & parameters)
  : ADVectorKernelValue<compute_stage>(parameters),
    _gravity_strong_residual(adGetADMaterialProperty<RealVectorValue>("gravity_strong_residual")),
    _mms_function_strong_residual(
        adGetADMaterialProperty<RealVectorValue>("mms_function_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
INSADMomentumForces<compute_stage>::precomputeQpResidual()
{
  return _gravity_strong_residual[_qp] + _mms_function_strong_residual[_qp];
}

/****************************************************************/
/**************** INSADMomentumSUPG *****************************/
/****************************************************************/

registerADMooseObject("NavierStokesApp", INSADMomentumSUPG);

defineADValidParams(
    INSADMomentumSUPG,
    ADVectorKernelSUPG,
    params.addClassDescription("Adds the supg stabilization to the INS momentum equation"););

template <ComputeStage compute_stage>
INSADMomentumSUPG<compute_stage>::INSADMomentumSUPG(const InputParameters & parameters)
  : ADVectorKernelSUPG<compute_stage>(parameters),
    _momentum_strong_residual(adGetADMaterialProperty<RealVectorValue>("momentum_strong_residual"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
INSADMomentumSUPG<compute_stage>::precomputeQpStrongResidual()
{
  return _momentum_strong_residual[_qp];
}
