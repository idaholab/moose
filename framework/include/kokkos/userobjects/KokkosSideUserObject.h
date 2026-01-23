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

#include "BoundaryRestrictableRequired.h"
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "ElementIDInterface.h"

namespace Moose::Kokkos
{

class SideUserObject : public UserObject,
                       public ::BoundaryRestrictableRequired,
                       public ::MaterialPropertyInterface,
                       public ::CoupleableMooseVariableDependencyIntermediateInterface,
                       public ::TransientInterface,
                       public ::ElementIDInterface
{
public:
  static InputParameters validParams();

  SideUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  SideUserObject(const SideUserObject & object);

  virtual void compute() override;
};

} // namespace Moose::Kokkos
