//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUResidualObject.h"

#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose
{
namespace Kokkos
{

class KernelBase : public ResidualObject,
                   public BlockRestrictable,
                   public CoupleableMooseVariableDependencyIntermediateInterface,
                   public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  KernelBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  KernelBase(const KernelBase & object);

protected:
  // Sets the variables this object depend on
  void setVariableDependency();
  // Sets the quadrature projection status flags for the variables, tags, and subdomains covered by
  // this object
  void setProjectionFlags();
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosKernelBaseMembers                                                               \
  usingKokkosResidualObjectMembers;                                                                \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::KernelBase::blockElementID;
