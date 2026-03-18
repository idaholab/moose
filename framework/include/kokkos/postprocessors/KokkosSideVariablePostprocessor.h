//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSidePostprocessor.h"

class KokkosSideVariablePostprocessor : public Moose::Kokkos::SidePostprocessor,
                                        public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  KokkosSideVariablePostprocessor(const InputParameters & parameters);

protected:
  /// Holds the solution at current quadrature points
  const Moose::Kokkos::VariableValue _u;

  /// Holds the solution gradient at the current quadrature points
  const Moose::Kokkos::VariableGradient _grad_u;
};
