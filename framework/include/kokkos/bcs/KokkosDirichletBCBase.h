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
#include "KokkosADNodalBC.h"

namespace Moose::Kokkos
{

/**
 * The base Kokkos boundary condition of a Dirichlet type
 */
template <bool is_ad>
class DirichletBCBaseTempl : public std::conditional_t<is_ad, ADNodalBC, NodalBC>
{
  using real_type = std::conditional_t<is_ad, ADReal, Real>;

public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  DirichletBCBaseTempl(const InputParameters & parameters);

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
   * Function tag for preset loop
   */
  struct PresetLoop
  {
  };

  /**
   * The preset function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(PresetLoop, const ThreadID tid, const Derived & bc) const;

  template <typename Derived>
  KOKKOS_FUNCTION auto computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

  using Base = std::conditional_t<is_ad, ADNodalBC, NodalBC>;
  using Base::operator();

protected:
  using Base::_kokkos_var;
  using Base::_u;
  using Base::kokkosAssembly;
  using Base::kokkosBoundaryNodeID;
  using Base::kokkosSystem;
  using Base::kokkosSystems;
  using Base::numKokkosBoundaryNodes;

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

template <bool is_ad>
template <typename Derived>
KOKKOS_FUNCTION void
DirichletBCBaseTempl<is_ad>::operator()(PresetLoop, const ThreadID tid, const Derived & bc) const
{
  auto node = kokkosBoundaryNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto dof = sys.getNodeLocalDofIndex(node, 0, _kokkos_var.var());

  if (dof == libMesh::DofObject::invalid_id)
    return;

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  sys.getVectorDofValue(dof, _solution_tag) = bc.computeValue(0, datum);
}

template <bool is_ad>
template <typename Derived>
KOKKOS_FUNCTION auto
DirichletBCBaseTempl<is_ad>::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);

  return _u(datum, qp) - real_type(bc->computeValue(qp, datum));
}

typedef DirichletBCBaseTempl<false> DirichletBCBase;
typedef DirichletBCBaseTempl<true> ADDirichletBCBase;

} // namespace Moose::Kokkos

#define registerKokkosDirichletBC(app, classname)                                                  \
  registerKokkosBoundaryCondition(app, classname);                                                 \
  registerKokkosAdditionalOperation(classname, PresetLoop)

#define registerKokkosADDirichletBC(app, classname)                                                \
  registerKokkosADBoundaryCondition(app, classname);                                               \
  registerKokkosAdditionalOperation(classname, PresetLoop)
