//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template <typename T, bool is_ad>
class MatDiffusionBaseTempl
  : public DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernelGrad<is_ad>>>
{
public:
  static InputParameters validParams();

  MatDiffusionBaseTempl(const InputParameters & parameters);

protected:
  virtual GenericRealVectorValue<is_ad> precomputeQpResidual() override;

  /// diffusion coefficient
  const GenericMaterialProperty<T, is_ad> & _diffusivity;

  /// diffusion coefficient derivative w.r.t. the kernel variable
  const MaterialProperty<T> * _ddiffusivity_dc;

  /// diffusion coefficient derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<T> *> _ddiffusivity_darg;

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  /// int label for the Concentration
  unsigned int _v_var;

  /// Gradient of the concentration
  const GenericVariableGradient<is_ad> & _grad_v;

  usingGenericKernelGradMembers;
};

template <typename T>
class MatDiffusionBase : public MatDiffusionBaseTempl<T, false>
{
public:
  using MatDiffusionBaseTempl<T, false>::MatDiffusionBaseTempl;

protected:
  virtual void initialSetup() override;
  virtual RealGradient precomputeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual RealGradient computeQpCJacobian();

  using MatDiffusionBaseTempl<T, false>::_diffusivity;
  using MatDiffusionBaseTempl<T, false>::_ddiffusivity_darg;
  using MatDiffusionBaseTempl<T, false>::_qp;
  using MatDiffusionBaseTempl<T, false>::_grad_phi;
  using MatDiffusionBaseTempl<T, false>::_j;
  using MatDiffusionBaseTempl<T, false>::_i;
  using MatDiffusionBaseTempl<T, false>::_ddiffusivity_dc;
  using MatDiffusionBaseTempl<T, false>::_phi;
  using MatDiffusionBaseTempl<T, false>::_grad_v;
  using MatDiffusionBaseTempl<T, false>::_grad_test;
  using MatDiffusionBaseTempl<T, false>::_v_var;
  using MatDiffusionBaseTempl<T, false>::_is_coupled;
};
