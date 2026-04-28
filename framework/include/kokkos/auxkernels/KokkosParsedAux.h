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
#include "KokkosParsedObjectBase.h"

/**
 * AuxKernel that evaluates a parsed function expression
 */
class KokkosParsedAux : public Moose::Kokkos::AuxKernel, public Moose::Kokkos::ParsedObjectBase
{
public:
  static InputParameters validParams();

  KokkosParsedAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, Datum & datum) const;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosParsedAux::computeValue(const unsigned int qp, Datum & datum) const
{
  return _evaluator.eval(_t, datum.q_point(qp), qp, &datum);
}
