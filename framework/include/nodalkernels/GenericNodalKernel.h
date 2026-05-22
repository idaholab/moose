//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"
#include "ADNodalKernel.h"

/**
 * Template for nodal kernels, using or not automatic differentiation
 */
template <bool is_ad>
class GenericNodalKernel : public NodalKernel
{
public:
  static InputParameters validParams() { return NodalKernel::validParams(); }
  GenericNodalKernel(const InputParameters & parameters) : NodalKernel(parameters) {}
};

template <>
class GenericNodalKernel<true> : public ADNodalKernel
{
public:
  static InputParameters validParams() { return ADNodalKernel::validParams(); }
  GenericNodalKernel(const InputParameters & parameters) : ADNodalKernel(parameters) {}

protected:
  virtual Real computeQpJacobian()
  {
    mooseError("I'm an AD object, so computeQpJacobian should never be called");
  }

  virtual Real computeQpOffDiagJacobian(unsigned int)
  {
    mooseError("I'm an AD object, so computeQpOffDiagJacobian should never be called");
  }
};

#define usingGenericNodalKernelMembers                                                             \
  usingPostprocessorInterfaceMembers;                                                              \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  usingTaggingInterfaceMembers;                                                                    \
  using GenericNodalKernel<is_ad>::_qp;                                                            \
  using GenericNodalKernel<is_ad>::_u;                                                             \
  using GenericNodalKernel<is_ad>::_var;                                                           \
  using GenericNodalKernel<is_ad>::getVar;                                                         \
  using Coupleable::coupled;                                                                       \
  using Coupleable::coupledComponents
