//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosAuxKernel.h"
#include "KokkosPolymorphicUserObjectBase.h"

class KokkosPolymorphicUserObjectAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosPolymorphicUserObjectAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int, Datum &) const
  {
    return _first.value(2.0) + _second.combine(3.0, 4.0, 2);
  }

private:
  using UserObject = Moose::Kokkos::VirtualUserObject<KokkosPolymorphicUserObjectBase>;

  const UserObject _first;
  const UserObject _second;
};
