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
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"

namespace Moose
{
namespace Kokkos
{

class ElementUserObject : public UserObject,
                          public ::BlockRestrictable,
                          public ::MaterialPropertyInterface,
                          public ::CoupleableMooseVariableDependencyIntermediateInterface,
                          public ::TransientInterface,
                          public ::RandomInterface,
                          public ::ElementIDInterface
{
public:
  static InputParameters validParams();

  ElementUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  ElementUserObject(const ElementUserObject & object);

  virtual void compute() override;
};

} // namespace Kokkos
} // namespace Moose
