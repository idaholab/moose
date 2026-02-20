//* This file is part of the MOOSE framework

#pragma once

#include "NodalKernel.h"
#include "ADNodalKernel.h"

#include <type_traits>

template <bool is_ad>
using GenericNodalKernelParent = typename std::conditional<is_ad, ADNodalKernel, NodalKernel>::type;

/**
 * Shared template base.
 * Inherits from either NodalKernel or ADNodalKernel depending on is_ad.
 * The using declarations bring _u, _qp, and paramError into scope here so that
 * the concrete non-AD subclass (GenericNodalKernel) inherits them automatically
 * without needing a macro.
 */
template <bool is_ad>
class GenericNodalKernelTempl : public GenericNodalKernelParent<is_ad>
{
public:
  static InputParameters validParams() { return GenericNodalKernelParent<is_ad>::validParams(); }
  GenericNodalKernelTempl(const InputParameters & parameters)
    : GenericNodalKernelParent<is_ad>(parameters)
  {
  }

protected:
  using GenericNodalKernelParent<is_ad>::_u;
  using GenericNodalKernelParent<is_ad>::_qp;
  using GenericNodalKernelParent<is_ad>::paramError;
};

/**
 * Concrete non-AD base class with Jacobian virtual methods.
 * Non-AD scalar nodal kernels should inherit from this class.
 */
class GenericNodalKernel : public GenericNodalKernelTempl<false>
{
public:
  GenericNodalKernel(const InputParameters & parameters)
    : GenericNodalKernelTempl<false>(parameters)
  {
  }

protected:
  virtual Real computeQpJacobian() override { return 0; }
  virtual void computeOffDiagJacobian(unsigned int jvar_num) override
  {
    NodalKernel::computeOffDiagJacobian(jvar_num);
  }
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override { return 0; }
};

/**
 * AD typedef.
 */
typedef GenericNodalKernelTempl<true> ADGenericNodalKernel;

/**
 * Selects the concrete base for the next level of inheritance.
 */
template <bool is_ad>
using GenericNodalKernelBase =
    typename std::conditional<is_ad, ADGenericNodalKernel, GenericNodalKernel>::type;
