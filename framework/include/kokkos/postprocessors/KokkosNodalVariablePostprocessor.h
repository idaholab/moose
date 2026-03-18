//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalPostprocessor.h"

class KokkosNodalVariablePostprocessor : public Moose::Kokkos::NodalPostprocessor,
                                         public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  KokkosNodalVariablePostprocessor(const InputParameters & parameters);

protected:
  const Moose::Kokkos::VariableValue _u;
};
