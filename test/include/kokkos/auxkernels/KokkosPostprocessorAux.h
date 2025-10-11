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
 * Testing object that just utilizes the value of a Postprocessor for the value of the Aux Variable
 */
class KokkosPostprocessorAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosPostprocessorAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  const Moose::Kokkos::PostprocessorValue _pp_val;
};

KOKKOS_FUNCTION inline Real
KokkosPostprocessorAux::computeValue(const unsigned int /* qp */, AssemblyDatum & /* datum */) const
{
  return _pp_val;
}
