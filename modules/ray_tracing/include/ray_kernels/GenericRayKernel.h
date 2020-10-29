//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayKernel.h"
#include "ADRayKernel.h"

template <bool is_ad>
class GenericRayKernel : public RayKernel
{
public:
  static InputParameters validParams() { return RayKernel::validParams(); };
  GenericRayKernel(const InputParameters & parameters) : RayKernel(parameters) {}
};

template <>
class GenericRayKernel<true> : public ADRayKernel
{
public:
  static InputParameters validParams() { return ADRayKernel::validParams(); };
  GenericRayKernel(const InputParameters & parameters) : ADRayKernel(parameters) {}

protected:
  /**
   * Dummy virtual that will never be called but is used in derived classes that
   * override computeJacobian() for the non-AD case.
   */
  virtual ADReal computeQpJacobian() { return 0; }

  /**
   * Dummy virtual that will never be called but is used in derived classes that
   * override computeQpOffDiagJacobian() for the non-AD case.
   */
  virtual ADReal computeQpOffDiagJacobian(const unsigned int) { return 0; }

  /**
   * Dummy virtual that will never be called but is used in derived classes that
   * override precalculateJacobian() for the non-AD case.
   */
  virtual void precalculateJacobian() {}

  /**
   * Dummy virtual that will never be called but is used in derived classes that
   * override precalculateOffDiagJacobian() for the non-AD case.
   */
  virtual void precalculateOffDiagJacobian(const unsigned int) {}
};

#define usingGenericRayKernelMembers                                                               \
  usingRayKernelBaseMembers;                                                                       \
  usingTaggingInterfaceMembers;                                                                    \
  using GenericRayKernel<is_ad>::_qp;                                                              \
  using GenericRayKernel<is_ad>::_i;                                                               \
  using GenericRayKernel<is_ad>::_j;                                                               \
  using GenericRayKernel<is_ad>::_u;                                                               \
  using GenericRayKernel<is_ad>::_phi;                                                             \
  using GenericRayKernel<is_ad>::_test;                                                            \
  using GenericRayKernel<is_ad>::_q_point;                                                         \
  using GenericRayKernel<is_ad>::_var;                                                             \
  using GenericRayKernel<is_ad>::_name;                                                            \
  using GenericRayKernel<is_ad>::getVar;                                                           \
  using Coupleable::coupled;                                                                       \
  using Coupleable::coupledComponents;                                                             \
  using Coupleable::coupledGenericValue
