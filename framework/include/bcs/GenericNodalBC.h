//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"
#include "ADNodalBC.h"

template <bool is_ad>
class GenericNodalBC : public NodalBC
{
public:
  static InputParameters validParams() { return NodalBC::validParams(); };
  GenericNodalBC(const InputParameters & parameters) : NodalBC(parameters) {}
};

template <>
class GenericNodalBC<true> : public ADNodalBC
{
public:
  static InputParameters validParams() { return ADNodalBC::validParams(); };
  GenericNodalBC(const InputParameters & parameters) : ADNodalBC(parameters) {}

protected:
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) = 0;
};

#define usingGenericNodalBCMembers                                                                 \
  usingPostprocessorInterfaceMembers;                                                              \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  usingTaggingInterfaceMembers;                                                                    \
  using GenericNodalBC<is_ad>::_qp;                                                                \
  using GenericNodalBC<is_ad>::coupledValue;                                                       \
  using GenericNodalBC<is_ad>::adCoupledValue;                                                     \
  using GenericNodalBC<is_ad>::_u;                                                                 \
  using GenericNodalBC<is_ad>::_var;                                                               \
  using GenericNodalBC<is_ad>::_name;                                                              \
  using GenericNodalBC<is_ad>::getVar;                                                             \
  using Coupleable::coupled;                                                                       \
  using Coupleable::coupledComponents
