//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"
#include "KokkosParsedObjectBase.h"

class KokkosParsedMaterial : public Moose::Kokkos::Material, public Moose::Kokkos::ParsedObjectBase
{
public:
  static InputParameters validParams();

  KokkosParsedMaterial(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

protected:
  /**
   * Parsed material property
   */
  Moose::Kokkos::MaterialProperty<Real> _prop;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosParsedMaterial::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  _prop(datum, qp) = _evaluator.eval(_t, datum.q_point(qp), qp, &datum);
}
