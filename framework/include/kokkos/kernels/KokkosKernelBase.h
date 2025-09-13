//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosResidualObject.h"

#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for Kokkos kernels
 */
class KernelBase : public ResidualObject,
                   public BlockRestrictable,
                   public CoupleableMooseVariableDependencyIntermediateInterface,
                   public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   */
  KernelBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  /**
   * Copy constructor for parallel dispatch
   */
  KernelBase(const KernelBase & object);
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosKernelBaseMembers                                                               \
  usingKokkosResidualObjectMembers;                                                                \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::KernelBase::kokkosBlockElementID
