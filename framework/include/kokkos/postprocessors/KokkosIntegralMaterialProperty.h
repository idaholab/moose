//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegralPostprocessor.h"
#include "KokkosMaterialPropertyValue.h"

template <typename Base>
class KokkosIntegralMaterialProperty : public Base
{
public:
  static InputParameters validParams();

  KokkosIntegralMaterialProperty(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpIntegral(const unsigned int qp, Datum & datum) const;

protected:
  const Moose::Kokkos::MaterialProperty<Real> _scalar;
};

template <typename Base>
KOKKOS_FUNCTION Real
KokkosIntegralMaterialProperty<Base>::computeQpIntegral(const unsigned int qp, Datum & datum) const
{
  return _scalar(datum, qp);
}

typedef KokkosIntegralMaterialProperty<KokkosElementIntegralPostprocessor>
    KokkosElementIntegralMaterialProperty;
typedef KokkosIntegralMaterialProperty<KokkosSideIntegralPostprocessor>
    KokkosSideIntegralMaterialProperty;
