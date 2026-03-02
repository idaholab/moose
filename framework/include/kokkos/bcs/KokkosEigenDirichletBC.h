//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBC.h"

/**
 * Set Dirichlet boundary condition for eigenvalue problems.
 * Value has to be zero.
 */
class KokkosEigenDirichletBC : public Moose::Kokkos::NodalBC
{
public:
  static InputParameters validParams();

  KokkosEigenDirichletBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    return 0;
  }
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    return 0;
  }
};
