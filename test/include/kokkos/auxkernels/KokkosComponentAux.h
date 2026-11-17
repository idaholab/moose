//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * Testing object that sets each of its variables to (c + 1) * (x + 1), where c is the index of the
 * variable in 'variables', so that every component of a multi-variable kernel gets a distinct field
 */
class KokkosComponentAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosComponentAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosComponentAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  return (datum.comp() + 1) * (datum.q_point(qp)(0) + 1);
}
