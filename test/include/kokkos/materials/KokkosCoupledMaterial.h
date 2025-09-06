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

/**
 * A material that couples a material property
 */
class KokkosCoupledMaterial : public Moose::Kokkos::Material<KokkosCoupledMaterial>
{
public:
  static InputParameters validParams();

  KokkosCoupledMaterial(const InputParameters & parameters);

  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const
  {
    _mat_prop(datum, qp) = 1.0;
  }
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    _mat_prop(datum, qp) =
        4.0 / _coupled_mat_prop(datum, qp); // This will produce a NaN if evaluated out of order
  }

protected:
  std::string _mat_prop_name;
  Moose::Kokkos::MaterialProperty<Real> _mat_prop;
  Moose::Kokkos::MaterialProperty<Real> _coupled_mat_prop;
};
