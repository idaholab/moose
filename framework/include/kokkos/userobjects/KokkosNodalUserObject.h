//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosUserObject.h"

#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"

namespace Moose
{
namespace Kokkos
{

class NodalUserObject : public UserObject,
                        public ::BlockRestrictable,
                        public ::BoundaryRestrictable,
                        public ::CoupleableMooseVariableDependencyIntermediateInterface,
                        public ::TransientInterface,
                        public ::RandomInterface
{
public:
  static InputParameters validParams();

  NodalUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  NodalUserObject(const NodalUserObject & object);

  virtual void compute() override;

protected:
  /**
   * Flag whether this object is boundary-restricted
   */
  const bool _bnd;
  /**
   * Flag for enable/disabling multiple execute calls on nodes that share block ids
   */
  const bool _unique_node_execute;
};

} // namespace Kokkos
} // namespace Moose
