//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUMaterial.h"

class GPUStatefulTest final : public GPUMaterial<GPUStatefulTest>
{
public:
  static InputParameters validParams();

  GPUStatefulTest(const InputParameters & parameters);

  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const
  {
    if (_coupled_val)
      for (unsigned int i = 0; i < _num_props; ++i)
        _properties[i](datum, qp) = _coupled_val(datum, qp);
    else
      for (unsigned int i = 0; i < _num_props; ++i)
        _properties[i](datum, qp) = _prop_values[i];
  }

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    // Really Expensive Fibonacci sequence generator!
    for (unsigned int i = 0; i < _num_props; ++i)
      _properties[i](datum, qp) = _properties_old[i](datum, qp) + _properties_older[i](datum, qp);
  }

private:
  // optional coupled variable
  GPUVariableValue _coupled_val;

  std::vector<std::string> _prop_names;
  GPUArray<Real> _prop_values;

  unsigned int _num_props;

  GPUArray<GPUMaterialProperty<Real>> _properties;
  GPUArray<GPUMaterialProperty<Real>> _properties_old;
  GPUArray<GPUMaterialProperty<Real>> _properties_older;
};
