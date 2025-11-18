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

class KokkosCoupledAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosCoupledAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, Datum & datum) const;

protected:
  const Real _value;
  // Enum stored as integer (0 : +, 1 : -, 2 : *, 3 : /)
  const int _operator;

  const unsigned int _coupled;
  const Moose::Kokkos::VariableValue _coupled_val;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledAux::computeValue(const unsigned int qp, Datum & datum) const
{
  if (_operator == 0)
    return _coupled_val(datum, qp) + _value;
  else if (_operator == 1)
    return _coupled_val(datum, qp) - _value;
  else if (_operator == 2)
    return _coupled_val(datum, qp) * _value;
  else if (_operator == 3)
  // We are going to do division for this operation
  // This is useful for testing evalutation order
  // when we attempt to divide by zero!
  {
    KOKKOS_ASSERT(_coupled_val(datum, qp) != 0);

    return _value / _coupled_val(datum, qp);
  }

  // Won't reach this statement
  return 0;
}
