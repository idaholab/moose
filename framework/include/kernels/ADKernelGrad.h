//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

template <typename>
class ADKernelGradTempl;

using ADKernelGrad = ADKernelGradTempl<Real>;
using ADVectorKernelGrad = ADKernelGradTempl<RealVectorValue>;

template <typename T>
class ADKernelGradTempl : public ADKernelTempl<T>
{
public:
  static InputParameters validParams();

  ADKernelGradTempl(const InputParameters & parameters);

  void computeResidual() override;
  void computeResidualAndJacobian() override;

protected:
  void computeResidualsForJacobian() override;

  /**
   * Called before forming the residual for an element
   */
  virtual typename OutputTools<typename Moose::ADType<T>::type>::OutputGradient
  precomputeQpResidual() = 0;

  virtual ADReal computeQpResidual() override final;

  using ADKernelTempl<T>::_assembly;
  using ADKernelTempl<T>::_var;
  using ADKernelTempl<T>::precalculateResidual;
  using ADKernelTempl<T>::_grad_test;
  using ADKernelTempl<T>::_use_displaced_mesh;
  using ADKernelTempl<T>::_qp;
  using ADKernelTempl<T>::_qrule;
  using ADKernelTempl<T>::_ad_JxW;
  using ADKernelTempl<T>::_ad_coord;
  using ADKernelTempl<T>::_i;
  using ADKernelTempl<T>::_JxW;
  using ADKernelTempl<T>::_coord;
  using ADKernelTempl<T>::_regular_grad_test;
  using ADKernelTempl<T>::accumulateTaggedLocalResidual;
  using ADKernelTempl<T>::_has_save_in;
  using ADKernelTempl<T>::_save_in;
  using ADKernelTempl<T>::_sys;
  using ADKernelTempl<T>::_j;
  using ADKernelTempl<T>::accumulateTaggedLocalMatrix;
  using ADKernelTempl<T>::prepareVectorTag;
  using ADKernelTempl<T>::_local_re;
  using ADKernelTempl<T>::_local_ke;
  using ADKernelTempl<T>::_diag_save_in;
  using ADKernelTempl<T>::_has_diag_save_in;
  using ADKernelTempl<T>::prepareMatrixTag;
  using ADKernelTempl<T>::_residuals;
};
