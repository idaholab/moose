//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalVariablePostprocessor.h"

class KokkosNodalMaxValueId : public KokkosNodalVariablePostprocessor
{
public:
  static InputParameters validParams();

  KokkosNodalMaxValueId(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

  template <typename Derived>
  KOKKOS_FUNCTION void
  executeShim(const Derived & postprocessor, Datum & datum, Real * result) const;

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, Datum & datum) const
  {
    return _u(datum, qp);
  }

  KOKKOS_FUNCTION void join(DefaultLoop, Real * result, const Real * source) const;
  KOKKOS_FUNCTION void init(DefaultLoop, Real * result) const;

protected:
  dof_id_type _node_id = libMesh::DofObject::invalid_id;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosNodalMaxValueId::executeShim(const Derived & postprocessor,
                                   Datum & datum,
                                   Real * result) const
{
  if (datum.isNodalDefined(_u.variable()))
  {
    Real value = postprocessor.computeValue(0, datum);

    if (value > result[0])
    {
      result[0] = value;
      result[1] = datum.node();
    }
  }
}

KOKKOS_FUNCTION inline void
KokkosNodalMaxValueId::join(DefaultLoop, Real * result, const Real * source) const
{
  if (source[0] > result[0])
  {
    result[0] = source[0];
    result[1] = source[1];
  }
}

KOKKOS_FUNCTION inline void
KokkosNodalMaxValueId::init(DefaultLoop, Real * result) const
{
  result[0] = -std::numeric_limits<Real>::max();
}
