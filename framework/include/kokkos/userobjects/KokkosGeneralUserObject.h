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

#include "ScalarCoupleable.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "DependencyResolverInterface.h"
#include "ReporterInterface.h"

namespace Moose
{
namespace Kokkos
{

class GeneralUserObject : public UserObject,
                          public ::ScalarCoupleable,
                          public ::MaterialPropertyInterface,
                          public ::TransientInterface
{
public:
  static InputParameters validParams();

  GeneralUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  GeneralUserObject(const GeneralUserObject & object);

  virtual void compute() override { execute(); }

  virtual void execute() = 0;
};

} // namespace Kokkos
} // namespace Moose
