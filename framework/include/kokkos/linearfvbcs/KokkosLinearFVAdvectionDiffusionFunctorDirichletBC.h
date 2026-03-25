//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVBoundaryCondition.h"
#include "KokkosFunction.h"

class KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
  : public Moose::Kokkos::LinearFVBoundaryCondition
{
public:
  static InputParameters validParams();

  KokkosLinearFVAdvectionDiffusionFunctorDirichletBC(const InputParameters & parameters);

  virtual void computeRightHandSide() override {}
  virtual void computeMatrix() override {}

  const Moose::Kokkos::Function & functor() const { return _functor; }

private:
  /// The functor providing the Dirichlet value(s) on a boundary
  const Moose::Kokkos::Function _functor;
};
