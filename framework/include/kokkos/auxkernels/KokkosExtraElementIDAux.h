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

class KokkosExtraElementIDAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosExtraElementIDAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

protected:
  const unsigned int _index;
};

KOKKOS_FUNCTION inline Real
KokkosExtraElementIDAux::computeValue(const unsigned int, AssemblyDatum & datum) const
{
  const dof_id_type id = datum.extraElemID(_index);

  return id == libMesh::DofObject::invalid_id ? -1.0 : id;
}
