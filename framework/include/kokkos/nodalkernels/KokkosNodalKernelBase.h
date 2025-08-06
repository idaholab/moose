//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosResidualObject.h"

#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for Kokkos nodal kernels
 */
class NodalKernelBase : public ResidualObject,
                        public BlockRestrictable,
                        public BoundaryRestrictable,
                        public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   */
  NodalKernelBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  /**
   * Copy constructor for parallel dispatch
   */
  NodalKernelBase(const NodalKernelBase & object);
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosNodalKernelBaseMembers                                                          \
  usingKokkosResidualObjectMembers;                                                                \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::NodalKernelBase::kokkosBlockNodeID;                                         \
  using Moose::Kokkos::NodalKernelBase::kokkosBoundaryNodeID
