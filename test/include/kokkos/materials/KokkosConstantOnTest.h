//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

class KokkosConstantOnTest : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosConstantOnTest(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    _property(datum, qp) = datum.elem().subdomain + 1;
  }

private:
  Moose::Kokkos::MaterialProperty<Real> _property;
};
