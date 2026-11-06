//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * Copies variables onto auxiliary variables, each source onto the auxiliary variable of the same
 * component
 */
class KokkosCopyValueAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosCopyValueAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeElementInternal(const Derived & auxkernel,
                                              AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeNodeInternal(const Derived & auxkernel, AssemblyDatum & datum) const;

protected:
  // Values are copied DOF by DOF without an L2 projection, so no mass matrix factors are needed
  virtual void setupMassFactors(const bool) override {}

  /// Variable used to specify state being copied
  unsigned short _state;

  /// The variable values to copy from
  const Moose::Kokkos::VariableValue _v;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosCopyValueAux::computeElementInternal(const Derived & /* auxkernel */,
                                           AssemblyDatum & datum) const
{
  auto & sys = kokkosSystem(_kokkos_var.sys(datum.comp()));
  auto var = _kokkos_var.var(datum.comp());
  auto tag = _kokkos_var.tag();
  auto elem = datum.elem().id;

  for (unsigned int i = 0; i < datum.n_dofs(); ++i)
    sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, i, var), tag) = _v(datum, i);
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosCopyValueAux::computeNodeInternal(const Derived & /* auxkernel */,
                                        AssemblyDatum & datum) const
{
  auto & sys = kokkosSystem(_kokkos_var.sys(datum.comp()));
  auto var = _kokkos_var.var(datum.comp());
  auto tag = _kokkos_var.tag();
  auto node = datum.node();

  sys.getVectorDofValue(sys.getNodeLocalDofIndex(node, 0, var), tag) = _v(datum, 0);
}
