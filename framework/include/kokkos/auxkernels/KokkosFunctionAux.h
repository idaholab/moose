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
#include "KokkosFunction.h"

class KokkosFunctionAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosFunctionAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _func.value(_t, datum.q_point(qp));
  }

protected:
  const Moose::Kokkos::Function _func;
};
