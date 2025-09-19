//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBC.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base Kokkos boundary condition of a Dirichlet type
 */
template <typename Derived>
class DirichletBCBase : public NodalBC
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  DirichletBCBase(const InputParameters & parameters);

  /**
   * Get whether the value is to be preset
   * @returns Whether the value is to be preset
   */
  virtual bool preset() const override { return _preset; }

  /**
   * Dispatch solution vector preset
   * @param tag The tag associated with the solution vector to be preset
   */
  virtual void presetSolution(TagID tag) override;

  /**
   * The preset function called by Kokkos
   */
  KOKKOS_FUNCTION void operator()(const ThreadID tid) const;

  /**
   * Compute residual contribution on a node
   * @param node The contiguous node ID
   * @returns The residual contribution
   */
  KOKKOS_FUNCTION Real computeQpResidual(const ContiguousNodeID node) const;

private:
  /**
   * Flag whether the value is to be preset
   */
  const bool _preset;
  /**
   * Tag associated with the solution vector to be preset
   */
  TagID _solution_tag;
};

template <typename Derived>
InputParameters
DirichletBCBase<Derived>::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

template <typename Derived>
DirichletBCBase<Derived>::DirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters), _preset(getParam<bool>("preset"))
{
}

template <typename Derived>
void
DirichletBCBase<Derived>::presetSolution(TagID tag)
{
  _solution_tag = tag;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>>(0, numKokkosBoundaryNodes()),
      *static_cast<Derived *>(this));
}

template <typename Derived>
KOKKOS_FUNCTION void
DirichletBCBase<Derived>::operator()(const ThreadID tid) const
{
  auto bc = static_cast<const Derived *>(this);
  auto node = kokkosBoundaryNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto dof = sys.getNodeLocalDofIndex(node, _kokkos_var.var());

  if (dof == libMesh::DofObject::invalid_id)
    return;

  sys.getVectorDofValue(dof, _solution_tag) = bc->computeValue(node);
}

template <typename Derived>
KOKKOS_FUNCTION Real
DirichletBCBase<Derived>::computeQpResidual(const ContiguousNodeID node) const
{
  auto bc = static_cast<const Derived *>(this);

  return _u(node) - bc->computeValue(node);
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosDirichletBCBaseMembers(T)                                                       \
public:                                                                                            \
  using Moose::Kokkos::DirichletBCBase<T>::operator();                                             \
  using Moose::Kokkos::NodalBC::operator();                                                        \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::NodalBC::_u
