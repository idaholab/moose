//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosGeneralUserObject.h"

class KokkosPolymorphicUserObjectBase : public Moose::Kokkos::GeneralUserObject
{
public:
  using GeneralUserObject::GeneralUserObject;

  KOKKOS_FUNCTION Real value(Real input) const;
  KOKKOS_FUNCTION Real combine(Real left, Real right, unsigned int component) const;
};

registerVirtualKokkosUserObjectBase(KokkosPolymorphicUserObjectBase, value, combine);
