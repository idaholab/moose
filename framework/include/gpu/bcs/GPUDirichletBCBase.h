//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUNodalBC.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base Kokkos boundary condition of a Dirichlet type
 */
template <typename Derived>
class DirichletBCBase : public NodalBC<Derived>
{
  usingKokkosNodalBCMembers(Derived);

public:
  static InputParameters validParams()
  {
    InputParameters params = NodalBC<Derived>::validParams();
    params.addParam<bool>(
        "preset",
        true,
        "Whether or not to preset the BC (apply the value before the solve begins).");
    return params;
  }

  /**
   * Constructor
   */
  DirichletBCBase(const InputParameters & parameters)
    : NodalBC<Derived>(parameters), _preset(this->template getParam<bool>("preset"))
  {
  }

  /**
   * Copy constructor for parallel dispatch
   */
  DirichletBCBase(const DirichletBCBase<Derived> & object)
    : NodalBC<Derived>(object), _preset(object._preset), _solution_tag(object._solution_tag)
  {
  }

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
  KOKKOS_FUNCTION void operator()(const size_t tid) const;

  /**
   * Compute residual contribution on a node
   * @param node The node ID
   * @returns The residual contribution
   */
  KOKKOS_FUNCTION Real computeQpResidual(const dof_id_type node) const
  {
    auto bc = static_cast<const Derived *>(this);

    return _u(node) - bc->computeValue(node);
  }

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
void
DirichletBCBase<Derived>::presetSolution(TagID tag)
{
  _solution_tag = tag;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<::Kokkos::IndexType<size_t>>(0, this->numBoundaryNodes()),
      *static_cast<Derived *>(this));
}

template <typename Derived>
KOKKOS_FUNCTION void
DirichletBCBase<Derived>::operator()(const size_t tid) const
{
  auto bc = static_cast<const Derived *>(this);
  auto node = boundaryNodeID(tid);

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto dof = sys.getNodeLocalDofIndex(node, _kokkos_var.var());

  sys.getVectorDofValue(dof, _solution_tag) = bc->computeValue(node);
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosDirichletBCBaseMembers(T)                                                       \
  usingKokkosNodalBCMembers(T);                                                                    \
                                                                                                   \
public:                                                                                            \
  using Moose::Kokkos::DirichletBCBase<T>::operator();
