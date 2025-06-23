//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMaterial.h"

class GPUGenericConstantMaterial final : public GPUMaterial<GPUGenericConstantMaterial>
{
public:
  static InputParameters validParams();

  GPUGenericConstantMaterial(const InputParameters & parameters);

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    for (unsigned int i = 0; i < _num_props; ++i)
    {
      auto prop = _props[i](datum, qp);
      prop = _prop_values[i];
    }
  }

protected:
  // Material property names
  const std::vector<std::string> & _prop_names;
  // GPU-accessible array of property values
  const GPUArray<Real> _prop_values;
  // GPU-accessible array of GPU material properties
  GPUArray<GPUMaterialProperty<Real>> _props;
  // Number of properties
  const unsigned int _num_props;
};
