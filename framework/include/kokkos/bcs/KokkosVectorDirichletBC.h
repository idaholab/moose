//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorNodalBC.h"

class KokkosVectorDirichletBC : public Moose::Kokkos::VectorNodalBC
{
public:
  static InputParameters validParams();

  KokkosVectorDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::Real3 computeQpResidual(const unsigned int qp,
                                                         AssemblyDatum & datum) const
  {
    return _u(datum, qp) - _values;
  }

protected:
  const Moose::Kokkos::Real3 _values;
};
