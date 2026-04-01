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

class KokkosOnDemandTest : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosOnDemandTest(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    unsigned int num_active = 0;

    for (unsigned int i = 0; i < _num_props; ++i)
      if (_properties[i])
        ++num_active;

    for (unsigned int i = 0; i < _num_props; ++i)
      if (_properties[i])
        _properties[i](datum, qp) = num_active;
  }

private:
  std::vector<std::string> _prop_names;

  unsigned int _num_props;

  Moose::Kokkos::Array<Moose::Kokkos::MaterialProperty<Real>> _properties;
};
