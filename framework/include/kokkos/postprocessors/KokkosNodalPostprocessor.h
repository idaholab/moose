//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosPostprocessor.h"
#include "KokkosNodalReducer.h"

namespace Moose
{
namespace Kokkos
{

class NodalPostprocessor : public NodalReducer, public Postprocessor
{
public:
  static InputParameters validParams();

  NodalPostprocessor(const InputParameters & parameters);

  /**
   * Finalize is not required for Postprocessor implementations since work may be done in
   * getValue().
   */
  virtual void finalize() override {}

  // Disambiguation with FunctorBase::operator()
  using NodalReducer::operator();
};

} // namespace Kokkos
} // namespace Moose
