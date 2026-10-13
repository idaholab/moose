//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDispatcher.h"
#include "KokkosFESystem.h"
#include "KokkosVariableValue.h"
#include "KokkosADAssembly.h"
#include "KokkosMaterialPropertyValue.h"
#include "KokkosConfiguration.h"

#include "MooseVariableBase.h"
#include "ResidualObject.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos residual objects
 */
class ResidualObject : public ::ResidualObject,
                       public MeshHolder,
                       public AssemblyHolder,
                       public FESystemHolder
{
public:
  static InputParameters validParams();

  /// Whether this object assembles the products of precompute hooks with the test functions
  static constexpr bool uses_precompute_hooks = false;

  /// Whether this object assembles off-diagonal Jacobian contributions of scalar variables
  static constexpr bool supports_scalar_jacobian = false;

  /// Whether this object dispatches its residual computation with one team per element or side
  static constexpr bool uses_team_dispatch = false;

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   * @param nodal Whether the residual object is a nodal residual object
   */
  ResidualObject(const InputParameters & parameters,
                 Moose::VarFieldType field_type,
                 bool nodal = false);
  /**
   * Copy constructor for parallel dispatch
   */
  ResidualObject(const ResidualObject & object);

  /**
   * Kokkos function tags
   */
  ///@{
  struct ResidualLoop
  {
  };
  struct JacobianLoop
  {
  };
  struct OffDiagJacobianLoop
  {
  };
  struct OffDiagJacobianScalarLoop
  {
  };
  ///@}

  virtual const MooseVariableBase & variable() const override { return _var; }

  virtual void computeOffDiagJacobian(unsigned int) override final
  {
    mooseError("computeOffDiagJacobian() is not used for Kokkos residual objects.");
  }
  virtual void computeResidualAndJacobian() override
  {
    computeResidual();
    computeJacobian();
  }

protected:
  /**
   * Reference of the MOOSE variable
   */
  MooseVariableFieldBase & _var;
  /**
   * Kokkos variable
   */
  Variable _kokkos_var;
  /**
   * Configuration on which the weak form is posed
   */
  Configuration _configuration;
  /**
   * Kokkos thread object
   */
  Thread<> _thread;
  /**
   * Kokkos functor dispatchers
   */
  ///@{
  std::unique_ptr<DispatcherBase> _residual_dispatcher;
  std::unique_ptr<DispatcherBase> _jacobian_dispatcher;
  std::unique_ptr<DispatcherBase> _offdiag_jacobian_dispatcher;
  std::unique_ptr<DispatcherBase> _offdiag_jacobian_scalar_dispatcher;
  ///@}

  /**
   * Mesh dimension
   */
  const unsigned int _dimension;

  /**
   * TODO: Move to TransientInterface
   */
  ///@{
  /**
   * Time
   */
  Scalar<Real> _t;
  /**
   * Old time
   */
  Scalar<const Real> _t_old;
  /**
   * The number of the time step
   */
  Scalar<int> _t_step;
  /**
   * Time step size
   */
  Scalar<Real> _dt;
  /**
   * Size of the old time step
   */
  Scalar<Real> _dt_old;
  ///@}

  /**
   * Accumulate local elemental residual contribution to tagged vectors
   * @param local_re The local elemental residual contribution
   * @param elem The contiguous element ID
   * @param i The test function index
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedElementalResidual(const Real local_re,
                                                         const ContiguousElementID elem,
                                                         const unsigned int i,
                                                         const unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal residual contribution to tagged vectors
   * @param add Whether to add or set the local residual
   * @param local_re The local nodal residual contribution
   * @param node The contiguous node ID
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedNodalResidual(const bool add,
                                                     const Real local_re,
                                                     const ContiguousNodeID node,
                                                     const unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal residual contribution to tagged vectors for vector FE variables
   * @param add Whether to add or set the local residual
   * @param local_re The local nodal residual contribution
   * @param node The contiguous node ID
   * @param i The vector component/node-local DOF index
   */
  KOKKOS_FUNCTION void accumulateTaggedVectorNodalResidual(const bool add,
                                                           const Real local_re,
                                                           const ContiguousNodeID node,
                                                           const unsigned int i) const;
  /**
   * Accumulate local elemental Jacobian contribution to tagged matrices
   * @param local_ke The local elemental Jacobian contribution
   * @param elem The contiguous element ID
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedElementalMatrix(const Real local_ke,
                                                       const ContiguousElementID elem,
                                                       const unsigned int i,
                                                       const unsigned int j,
                                                       const unsigned int jvar,
                                                       const unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal Jacobian contribution to tagged matrices
   * @param add Whether to add or set the local Jacobian
   * @param local_ke The local nodal Jacobian contribution
   * @param node The contiguous node ID
   * @param jvar The variable number for column
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedNodalMatrix(const bool add,
                                                   const Real local_ke,
                                                   const ContiguousNodeID node,
                                                   const unsigned int jvar,
                                                   const unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal Jacobian contribution to tagged matrices for vector FE variables
   * @param add Whether to add or set the local Jacobian
   * @param local_ke The local nodal Jacobian contribution
   * @param node The contiguous node ID
   * @param i The vector component/node-local DOF index for the row
   * @param j The node-local DOF index for the column
   * @param jvar The variable number for column
   */
  KOKKOS_FUNCTION void accumulateTaggedVectorNodalMatrix(const bool add,
                                                         const Real local_ke,
                                                         const ContiguousNodeID node,
                                                         const unsigned int i,
                                                         const unsigned int j,
                                                         const unsigned int jvar) const;

  /**
   * The common loop structure template for computing elemental residual
   * @param datum The AssemblyDatum object of the current thread
   * @param body The quadrature point loop body
   */
  template <typename function>
  KOKKOS_FUNCTION void computeResidualInternal(AssemblyDatum & datum, function body) const;
  /**
   * The common loop structure template for computing elemental Jacobian
   * @param datum The AssemblyDatum object of the current thread
   * @param body The quadrature point loop body
   */
  template <typename function>
  KOKKOS_FUNCTION void computeJacobianInternal(AssemblyDatum & datum, function body) const;

  /**
   * Assemble the residual and Jacobian of a pointwise AD residual object on one element or side
   * with one team. The team evaluates the residual hooks once per quadrature point into scratch,
   * then contracts the stored coefficients with the shape functions:
   *
   *   R_i    = sum_qp (f_v psi_i + f_g . grad psi_i)
   *   J_ij^b = sum_qp (df_v/ds psi_i + df_g/ds . grad psi_i) dq_s/dU_j
   *
   * where slot block b of a variable holds its value (dq/dU_j = phi_j) or its gradient components
   * (dq_k/dU_j = d phi_j / d x_k), and column j indexes the DOFs of that variable.
   * @param object The residual object of the final derived type
   * @param team The team handle
   * @param datum The AssemblyDatum object of the element or side
   * @param table The slot table of the residual object
   * @param computing_residual Whether to assemble the residual
   * @param computing_jacobian Whether to assemble the Jacobian
   */
  template <typename Object, typename Team>
  KOKKOS_FUNCTION void assembleADTerms(const Object & object,
                                       const Team & team,
                                       AssemblyDatum & datum,
                                       const ADSlotTable & table,
                                       bool computing_residual,
                                       bool computing_jacobian) const;

private:
  /**
   * Tags this object operates on
   */
  ///@{
  Array<TagID> _vector_tags;
  Array<TagID> _matrix_tags;
  ///@}
};

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedElementalResidual(const Real local_re,
                                                  const ContiguousElementID elem,
                                                  const unsigned int i,
                                                  const unsigned int comp) const
{
  if (!local_re)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  const auto var = _kokkos_var.var(comp);
  auto dof = sys.getElemLocalDofIndex(elem, i, var);
  const auto scaled_local_re = local_re * sys.getVariableScalingFactor(var);

  for (unsigned int t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag))
      ::Kokkos::atomic_add(&sys.getVectorDofValue(dof, tag), scaled_local_re);
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedNodalResidual(const bool add,
                                              const Real local_re,
                                              const ContiguousNodeID node,
                                              const unsigned int comp) const
{
  if (!local_re && add)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto dof = sys.getNodeLocalDofIndex(node, 0, _kokkos_var.var(comp));

  for (unsigned int t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag))
    {
      if (add)
        sys.getVectorDofValue(dof, tag) += local_re;
      else
        sys.getVectorDofValue(dof, tag) = local_re;
    }
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedVectorNodalResidual(const bool add,
                                                    const Real local_re,
                                                    const ContiguousNodeID node,
                                                    const unsigned int i) const
{
  if (!local_re && add)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto dof = sys.getNodeLocalDofIndex(node, i, _kokkos_var.var());

  for (unsigned int t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag))
    {
      if (add)
        sys.getVectorDofValue(dof, tag) += local_re;
      else
        sys.getVectorDofValue(dof, tag) = local_re;
    }
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedElementalMatrix(const Real local_ke,
                                                const ContiguousElementID elem,
                                                const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int comp) const
{
  if (!local_ke)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  const auto ivar = _kokkos_var.var(comp);
  auto row = sys.getElemLocalDofIndex(elem, i, ivar);
  auto col = sys.isScalarVariable(jvar) ? sys.getScalarGlobalDofIndex(j, jvar)
                                        : sys.getElemGlobalDofIndex(elem, j, jvar);
  const auto scaled_local_ke = local_ke * sys.getVariableScalingFactor(ivar);

  for (unsigned int t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag) && !sys.hasNodalBCMatrixTag(row, tag))
      ::Kokkos::atomic_add(&sys.getMatrixValue(row, col, tag), scaled_local_ke);
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedNodalMatrix(const bool add,
                                            const Real local_ke,
                                            const ContiguousNodeID node,
                                            const unsigned int jvar,
                                            const unsigned int comp) const
{
  if (!local_ke && add)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto row = sys.getNodeLocalDofIndex(node, 0, _kokkos_var.var(comp));
  auto col = sys.getNodeGlobalDofIndex(node, 0, jvar);

  for (unsigned int t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag))
    {
      auto & matrix = sys.getMatrix(tag);

      if (add)
        matrix(row, col) += local_ke;
      else
      {
        matrix.zero(row);
        matrix(row, col) = local_ke;
      }
    }
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedVectorNodalMatrix(const bool add,
                                                  const Real local_ke,
                                                  const ContiguousNodeID node,
                                                  const unsigned int i,
                                                  const unsigned int j,
                                                  const unsigned int jvar) const
{
  if (!local_ke && add)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto row = sys.getNodeLocalDofIndex(node, i, _kokkos_var.var());
  auto col = sys.getNodeGlobalDofIndex(node, j, jvar);

  for (unsigned int t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag))
    {
      auto & matrix = sys.getMatrix(tag);

      if (add)
        matrix(row, col) += local_ke;
      else
      {
        matrix.zero(row);
        matrix(row, col) = local_ke;
      }
    }
  }
}

template <typename function>
KOKKOS_FUNCTION void
ResidualObject::computeResidualInternal(AssemblyDatum & datum, function body) const
{
  Real local_re[MAX_CACHED_DOF];

  unsigned int stride = MAX_CACHED_DOF * datum.num_local_threads();
  unsigned int num_batches = datum.n_dofs() / stride;

  if (datum.n_dofs() % stride)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ib = batch * stride;
    unsigned int ie = ::Kokkos::min(ib + stride, datum.n_dofs());

    const unsigned int n = ie - ib;
    const unsigned int d = n / datum.num_local_threads();
    const unsigned int m = n % datum.num_local_threads();
    const unsigned int t = datum.local_thread_id();

    ib += t * d + (t < m ? t : m);
    ie = ib + d + (t < m ? 1 : 0);

    for (unsigned int i = ib; i < ie; ++i)
      local_re[i - ib] = 0;

    body(local_re - ib, ib, ie);

    for (unsigned int i = ib; i < ie; ++i)
      accumulateTaggedElementalResidual(local_re[i - ib], datum.elem().id, i);
  }
}

template <typename function>
KOKKOS_FUNCTION void
ResidualObject::computeJacobianInternal(AssemblyDatum & datum, function body) const
{
  Real local_ke[MAX_CACHED_DOF];

  for (unsigned int j = datum.local_thread_id(); j < datum.n_jdofs();
       j += datum.num_local_threads())
  {
    unsigned int num_batches = datum.n_idofs() / MAX_CACHED_DOF;

    if (datum.n_idofs() % MAX_CACHED_DOF)
      ++num_batches;

    for (unsigned int batch = 0; batch < num_batches; ++batch)
    {
      unsigned int ib = batch * MAX_CACHED_DOF;
      unsigned int ie = ::Kokkos::min(ib + MAX_CACHED_DOF, datum.n_idofs());

      for (unsigned int i = ib; i < ie; ++i)
        local_ke[i - ib] = 0;

      body(local_ke - ib, ib, ie, j);

      for (unsigned int i = ib; i < ie; ++i)
        accumulateTaggedElementalMatrix(local_ke[i - ib], datum.elem().id, i, j, datum.jvar());
    }
  }
}

template <typename Object, typename Team>
KOKKOS_FUNCTION void
ResidualObject::assembleADTerms(const Object & object,
                                const Team & team,
                                AssemblyDatum & datum,
                                const ADSlotTable & table,
                                const bool computing_residual,
                                const bool computing_jacobian) const
{
  constexpr auto slots = Object::max_ad_slots;
  constexpr auto qp_size = ADScratchLayout::qpSize(slots);
  constexpr auto term_size = ADScratchLayout::term_size;

  const auto n_qps = datum.n_qps();
  ADScratchView scratch(team.team_scratch(0), n_qps * qp_size);

  ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_qps),
                         [&](const unsigned int qp)
                         { evaluateADTerms(object, datum, qp, &scratch(qp * qp_size)); });
  team.team_barrier();

  const auto & assembly = datum.assembly();
  const auto & elem = datum.elem();
  const auto side = datum.side();
  const auto ife = datum.ife();
  const auto n_idofs = datum.n_idofs();
  const auto & sys = kokkosSystem(datum.sys());

  // Shape function value and physical gradient of an FE type at a quadrature point
  auto shape = [&](const unsigned int fe, const unsigned int i, const unsigned int qp)
  {
    const Real phi = side == libMesh::invalid_uint
                         ? assembly.getPhi(elem.subdomain, elem.type, fe)(i, qp)
                         : assembly.getPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
    const Real3 ref = side == libMesh::invalid_uint
                          ? assembly.getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                          : assembly.getGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
    return ::Kokkos::pair<Real, Real3>(phi, datum.dxi_dx(qp) * ref);
  };

  if (computing_residual)
    ::Kokkos::parallel_for(::Kokkos::TeamThreadRange(team, n_idofs),
                           [&](const unsigned int i)
                           {
                             Real local_re = 0;
                             for (unsigned int qp = 0; qp < n_qps; ++qp)
                             {
                               const auto psi = shape(ife, i, qp);
                               local_re +=
                                   contractADTerms(&scratch(qp * qp_size), psi.first, psi.second);
                             }
                             accumulateTaggedElementalResidual(local_re, elem.id, i);
                           });

  if constexpr (slots > 0)
    if (computing_jacobian)
      for (unsigned int b = 0; b < table.numBlocks(); ++b)
      {
        const auto & block = table.block(b);
        const auto jfe = sys.getFETypeID(block.var);
        const auto n_jdofs = assembly.getNumDofs(elem.type, jfe);

        ::Kokkos::parallel_for(
            ::Kokkos::TeamThreadRange(team, n_idofs * n_jdofs),
            [&](const unsigned int ij)
            {
              const auto i = ij / n_jdofs;
              const auto j = ij % n_jdofs;
              Real local_ke = 0;

              for (unsigned int qp = 0; qp < n_qps; ++qp)
              {
                const auto test = shape(ife, i, qp);
                const auto trial = shape(jfe, j, qp);
                const Real * q = &scratch(qp * qp_size);

                if (block.gradient)
                  for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
                    local_ke += contractADTerms(q + term_size * (1 + block.first + k),
                                                test.first,
                                                test.second) *
                                trial.second(k);
                else
                  local_ke +=
                      contractADTerms(q + term_size * (1 + block.first), test.first, test.second) *
                      trial.first;
              }

              accumulateTaggedElementalMatrix(local_ke, elem.id, i, j, block.var);
            });
      }
}

} // namespace Moose::Kokkos
