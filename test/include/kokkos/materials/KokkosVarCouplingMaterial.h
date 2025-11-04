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
 * A material that couples a variable
 */
class KokkosVarCouplingMaterial : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosVarCouplingMaterial(const InputParameters & parameters);

  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const
  {
    _coupled_prop(datum, qp) = _var(datum, qp);
  }
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    // If "declare_old" is set, then just use it. The test associated is checking that
    // initQpStatefulProperties can use a coupledValue
    if (_coupled_prop_old)
      _coupled_prop(datum, qp) = _coupled_prop_old(datum, qp);
    else
      _coupled_prop(datum, qp) = _base + _coef * _var(datum, qp);
  }

protected:
  const Moose::Kokkos::VariableValue _var;
  Real _base;
  Real _coef;
  Moose::Kokkos::MaterialProperty<Real> _coupled_prop;
  Moose::Kokkos::MaterialProperty<Real> _coupled_prop_old;
};
