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
#include "KokkosSideReducer.h"

namespace Moose::Kokkos
{

class SidePostprocessor : public SideReducer, public Postprocessor
{
public:
  static InputParameters validParams();

  SidePostprocessor(const InputParameters & parameters);

  /**
   * We provide default finalize() as getValue() has been abused to perform the final aggregation
   * for a long time and we allowed not implementing finalize(). However, it is desired to do all
   * the finalization work such as communication in finalize() and let getValue() simply return the
   * final aggregated value, as getValue() is designed to be a const method.
   */
  virtual void finalize() override {}

  // Disambiguation with FunctorBase::operator()
  using SideReducer::operator();
};

} // namespace Moose::Kokkos
