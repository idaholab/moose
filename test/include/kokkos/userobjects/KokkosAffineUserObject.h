//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosPolymorphicUserObjectBase.h"

class KokkosAffineUserObject : public KokkosPolymorphicUserObjectBase
{
public:
  static InputParameters validParams();

  KokkosAffineUserObject(const InputParameters & parameters);

  void initialize() override {}
  void finalize() override {}
  void compute() override {}

  KOKKOS_FUNCTION Real value(Real input) const { return _factor * input + _offset; }

  KOKKOS_FUNCTION Real combine(Real left, Real right, unsigned int component) const
  {
    return value(left) + component * right;
  }

private:
  const Real _factor;
  const Real _offset;
};
