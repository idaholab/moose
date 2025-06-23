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

class GPUStatefulSpatialTest final : public GPUMaterial<GPUStatefulSpatialTest>
{
public:
  static InputParameters validParams();

  GPUStatefulSpatialTest(const InputParameters & parameters);

  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const
  {
    _thermal_conductivity(datum, qp) = _t_step + (datum.q_point(qp)(0) * datum.q_point(qp)(1));
  }

  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    _thermal_conductivity(datum, qp) = _thermal_conductivity_old(datum, qp) + 1.;
  }

protected:
  GPUMaterialProperty<Real> _thermal_conductivity;
  GPUMaterialProperty<Real> _thermal_conductivity_old;
};
