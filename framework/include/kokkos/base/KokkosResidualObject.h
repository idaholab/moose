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
#include "KokkosPrecomputeHooks.h"

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

  /// Whether this object dispatches its residual computation with teams of elements or sides
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
  /// Jacobian with respect to one displacement component of an object posed on the current
  /// configuration, dispatched with one team per element or side
  struct DisplacementJacobianLoop
  {
  };
  ///@}
  /**
   * Kokkos function tag of a loop of a precompute or AD object compiled for one configuration.
   * The reference instantiation compiles out the displaced geometry of its Datum.
   * @tparam Loop The function tag of the loop
   * @tparam displaced Whether the loop runs on the current configuration
   */
  template <typename Loop, bool displaced>
  struct ConfigurationLoop
  {
  };

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
  /// Dispatcher of the residual and Jacobian of a pointwise AD object
  std::unique_ptr<DispatcherBase> _residual_and_jacobian_dispatcher;
  /// Dispatcher of the residual and Jacobian of a pointwise AD object when a displacement
  /// component is seeded
  std::unique_ptr<DispatcherBase> _displacement_seeded_dispatcher;
  /// Dispatcher of the Jacobian with respect to the displacements of a precompute object on the
  /// current configuration
  std::unique_ptr<DispatcherBase> _displacement_jacobian_dispatcher;
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
   * with one group of a team. The group evaluates the residual hooks once per quadrature point into
   * its scratch, then contracts the stored coefficients with the shape functions:
   *
   *   R_i = sum_qp (f_v psi_i + f_g . grad_xi psi_i)
   *
   * where f_g is the gradient coefficient pulled back to the reference element. The Jacobian with
   * respect to the DOFs j of a column variable contracts in two stages:
   *
   *   C_t(qp, j) = sum_s df_t/ds dq_s/dU_j
   *   J_ij       = sum_qp (C_v(qp, j) psi_i + C_g(qp, j) . grad_xi psi_i)
   *
   * where the slots s of the variable hold its value (dq/dU_j = phi_j) or its gradient on the
   * reference configuration, whose derivatives the scratch stores mapped to the reference element
   * (dq_k/dU_j = d phi_j / d xi_k). Each entry of a column accumulates once per chunk of quadrature
   * points.
   * @tparam slots The number of dual slots of the instantiation
   * @tparam displaced Whether the instantiation carries the displacement derivatives. Its dual
   * slots then hold intermediate quantities, which the chain rule maps to the true slots.
   * @param object The residual object of the final derived type
   * @param team The team handle
   * @param group The group of the team that assembles the element or side
   * @param datum The AD datum of the element or side for the current thread. The displaced
   * instantiation needs its chain record.
   * @param table The slot table of the residual object
   * @param computing_residual Whether to assemble the residual
   * @param computing_jacobian Whether to assemble the Jacobian
   *
   * The function is force-inlined, together with evaluateADTerms. The compiler declines to inline
   * it because of its size. A separate device function would take the AD datum by reference and
   * place it in local memory.
   */
  template <unsigned int slots, bool displaced, typename Object, typename Team>
  KOKKOS_FORCEINLINE_FUNCTION void assembleADTerms(const Object & object,
                                       const Team & team,
                                       const ADTeamGroup & group,
                                       ADAssemblyDatum<slots> & datum,
                                       const ADSlotTable & table,
                                       bool computing_residual,
                                       bool computing_jacobian) const;

  /**
   * Dispatch a team operation of a pointwise AD residual object, whose teams hold groups of
   * threads that each assemble one element or side
   * @param dispatcher The team dispatcher
   * @param n_items The number of elements or sides
   * @param max_qps The maximum number of quadrature points of an element or side
   * @param slots The number of true slots of the scratch layout
   * @param table The slot table of the object, whose column variables size the Jacobian
   * contraction
   */
  void dispatchADTeams(DispatcherBase & dispatcher,
                       ThreadID n_items,
                       unsigned int max_qps,
                       unsigned int slots,
                       const ADSlotTable & table);

  /**
   * Scratch layout of the current dispatch of a pointwise AD residual object
   */
  ADScratchLayout _ad_layout;

  /**
   * Displacement component assembled by the current dispatch of DisplacementJacobianLoop
   */
  unsigned int _displacement_component = 0;

  /**
   * Dispatch the Jacobian with respect to each displacement component of a precompute object posed
   * on the current configuration, one team per element or side
   * @param league_size The number of elements or sides
   */
  void dispatchDisplacementJacobian(ThreadID league_size, bool vector);

  /**
   * Build the dispatcher of a loop of a precompute or AD object compiled for its configuration
   * @tparam Loop The function tag of the loop
   * @param object The pointer to the functor of the final derived type
   * @returns The dispatcher
   */
  template <typename Loop>
  std::unique_ptr<DispatcherBase> buildConfigurationDispatcher(const void * object) const;
  /**
   * Get whether a precompute object defines the hooks that a loop assembles
   * @tparam Loop The function tag of the loop
   * @returns Whether the object defines the hooks
   */
  template <typename Loop>
  bool hasConfigurationUserMethod() const
  {
    return DispatcherRegistry::hasUserMethod<ConfigurationLoop<Loop, false>>(type());
  }

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
  auto dof = sys.getElemLocalDofIndex(elem, i, _kokkos_var.var(comp));
  auto constraints = sys.getLocalConstraints(dof);

  for (unsigned int t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag))
    {
      if (constraints.size())
        for (unsigned int c = 0; c < constraints.size(); ++c)
        {
          auto [constraint_dof, weight] = constraints[c];

          ::Kokkos::atomic_add(&sys.getVectorDofValue(constraint_dof, tag), weight * local_re);
        }
      else
        ::Kokkos::atomic_add(&sys.getVectorDofValue(dof, tag), local_re);
    }
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
  auto col = sys.isScalarVariable(jvar) ? sys.getScalarLocalDofIndex(j, jvar)
                                        : sys.getElemLocalDofIndex(elem, j, jvar);
  auto col_global = sys.localToGlobalDofIndex(col);
  auto row_constraints = sys.getLocalConstraints(row);
  auto col_constraints = sys.getLocalConstraints(col);

  for (unsigned int t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag))
    {
      for (unsigned int rc = 0; rc < ::Kokkos::max(1u, row_constraints.size()); ++rc)
      {
        auto constraint_row = row_constraints.size() ? row_constraints[rc].first : row;
        auto weight_row = row_constraints.size() ? row_constraints[rc].second : 1.0;

        if (sys.hasNodalBCMatrixTag(constraint_row, tag))
          continue;

        for (unsigned int cc = 0; cc < ::Kokkos::max(1u, col_constraints.size()); ++cc)
        {
          auto constraint_col = col_constraints.size()
                                    ? sys.localToGlobalDofIndex(col_constraints[cc].first)
                                    : col_global;
          auto weight_col = col_constraints.size() ? col_constraints[cc].second : 1.0;

          ::Kokkos::atomic_add(&sys.getMatrixValue(constraint_row, constraint_col, tag),
                               weight_row * weight_col * local_ke);
        }
      }
    }
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

template <typename Loop>
std::unique_ptr<DispatcherBase>
ResidualObject::buildConfigurationDispatcher(const void * object) const
{
  return _configuration.displaced()
             ? DispatcherRegistry::build<ConfigurationLoop<Loop, true>>(object, type())
             : DispatcherRegistry::build<ConfigurationLoop<Loop, false>>(object, type());
}

template <typename function>
KOKKOS_FUNCTION void
ResidualObject::computeResidualInternal(AssemblyDatum & datum, function body) const
{
  Real local_re[MAX_CACHED_DOF];

  const auto scaling_factor =
      kokkosSystem(_kokkos_var.sys()).getVariableScalingFactor(_kokkos_var.var());

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
      accumulateTaggedElementalResidual(local_re[i - ib] * scaling_factor, datum.elem().id, i);
  }
}

template <typename function>
KOKKOS_FUNCTION void
ResidualObject::computeJacobianInternal(AssemblyDatum & datum, function body) const
{
  Real local_ke[MAX_CACHED_DOF];

  const auto scaling_factor =
      kokkosSystem(_kokkos_var.sys()).getVariableScalingFactor(_kokkos_var.var());

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
        accumulateTaggedElementalMatrix(
            local_ke[i - ib] * scaling_factor, datum.elem().id, i, j, datum.jvar());
    }
  }
}

template <unsigned int slots, bool displaced, typename Object, typename Team>
KOKKOS_FORCEINLINE_FUNCTION void
ResidualObject::assembleADTerms(const Object & object,
                                const Team & team,
                                const ADTeamGroup & group,
                                ADAssemblyDatum<slots> & datum,
                                const ADSlotTable & table,
                                const bool computing_residual,
                                const bool computing_jacobian) const
{
  constexpr auto term_size = ADScratchLayout::term_size;

  // The scratch holds the derivatives with respect to the true slots. Outside the displaced
  // instantiation, the dual slots are the true slots.
  const auto & layout = _ad_layout;
  KOKKOS_ASSERT(displaced || layout.slots == slots);
  KOKKOS_ASSERT(displaced == (datum.chain() != nullptr));
  const auto qp_size = ADScratchLayout::qpSize(displaced ? layout.slots : slots);

  const auto n_qps = datum.n_qps();
  const auto group_scratch_size = layout.groupSize();
  const ADScratchView team_scratch(team.team_scratch(0), group.count() * group_scratch_size);
  const ADScratchView scratch(&team_scratch(group.index() * group_scratch_size),
                              group_scratch_size);

  // The threads of the group stride over a range, which a group without an element or side skips
  auto group_for = [&](const unsigned int n, auto && body)
  {
    if (group.active())
      for (unsigned int k = group.rank(); k < n; k += group.size())
        body(k);
  };

  // Each thread evaluates its quadrature points with its own datum, whose geometry cache is
  // thread-private
  if (group.active())
    for (unsigned int qp = group.rank(); qp < n_qps; qp += group.size())
      evaluateADTerms<displaced, slots>(
          object, datum, table, layout.slots, qp, &scratch(qp * qp_size));
  team.team_barrier();

  const auto & assembly = datum.assembly();
  const auto & elem = datum.elem();
  const auto side = datum.side();
  const auto ife = datum.ife();
  const auto n_idofs = datum.n_idofs();
  const auto & sys = kokkosSystem(datum.sys());
  const auto scaling_factor =
      kokkosSystem(_kokkos_var.sys()).getVariableScalingFactor(_kokkos_var.var());

  // Shape function value and reference gradient of an FE type at a quadrature point
  auto shape = [&](const unsigned int fe, const unsigned int i, const unsigned int qp)
  {
    const Real phi = side == libMesh::invalid_uint
                         ? assembly.getPhi(elem.subdomain, elem.type, fe)(i, qp)
                         : assembly.getPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
    const Real3 ref = side == libMesh::invalid_uint
                          ? assembly.getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                          : assembly.getGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
    return ::Kokkos::pair<Real, Real3>(phi, ref);
  };

  if (computing_residual)
    group_for(n_idofs,
              [&](const unsigned int i)
              {
                Real local_re = 0;
                for (unsigned int qp = 0; qp < n_qps; ++qp)
                {
                  const auto psi = shape(ife, i, qp);
                  local_re += contractADTerms(&scratch(qp * qp_size), psi.first, psi.second);
                }
                accumulateTaggedElementalResidual(local_re * scaling_factor, elem.id, i);
              });

  if constexpr (slots > 0)
    if (computing_jacobian)
    {
      Real * const planes = &scratch(layout.jacobianOffset());
      const auto plane = layout.jacobianPlane();
      const auto chunk = layout.qp_chunk;
      KOKKOS_ASSERT(chunk > 0);
      // Every group runs the same number of chunks, so that the team barriers match
      const auto n_chunks = (layout.max_qps + chunk - 1) / chunk;

      for (unsigned int b = 0; b < table.numBlocks(); ++b)
      {
        // The first block of a variable leads its column
        const auto & lead = table.block(b);
        bool leads = true;
        for (unsigned int c = 0; c < b; ++c)
          if (table.block(c).var == lead.var)
            leads = false;
        if (!leads)
          continue;

        const auto jfe = sys.getFETypeID(lead.var);
        const auto n_jdofs = assembly.getNumDofs(elem.type, jfe);

        for (unsigned int ch = 0; ch < n_chunks; ++ch)
        {
          const auto qp_begin = ch * chunk;
          const auto n_chunk_qps = qp_begin < n_qps ? ::Kokkos::min(chunk, n_qps - qp_begin) : 0u;

          // Stage 1: contract the slot derivatives of the column with the trial functions
          group_for(n_chunk_qps * n_jdofs,
                    [&](const unsigned int qj)
                    {
                      const auto q = qj / n_jdofs;
                      const auto j = qj % n_jdofs;
                      const auto trial = shape(jfe, j, qp_begin + q);
                      const Real * const derivatives = &scratch((qp_begin + q) * qp_size);
                      Real c[term_size] = {0, 0, 0, 0};

                      for (unsigned int s = b; s < table.numBlocks(); ++s)
                      {
                        const auto & block = table.block(s);
                        if (block.var != lead.var)
                          continue;

                        const Real * const d = derivatives + term_size * (1 + block.first);
                        if (block.gradient)
                          for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
                            for (unsigned int t = 0; t < term_size; ++t)
                              c[t] += d[term_size * k + t] * trial.second(k);
                        else
                          for (unsigned int t = 0; t < term_size; ++t)
                            c[t] += d[t] * trial.first;
                      }

                      for (unsigned int t = 0; t < term_size; ++t)
                        planes[t * plane + qj] = c[t];
                    });
          team.team_barrier();

          // Stage 2: contract with the test functions
          if (n_chunk_qps)
            group_for(n_idofs * n_jdofs,
                      [&](const unsigned int ij)
                      {
                        const auto i = ij / n_jdofs;
                        const auto j = ij % n_jdofs;
                        Real local_ke = 0;

                        for (unsigned int q = 0; q < n_chunk_qps; ++q)
                        {
                          const auto test = shape(ife, i, qp_begin + q);
                          const Real * const cq = planes + q * n_jdofs + j;
                          local_ke += cq[0] * test.first + cq[plane] * test.second(0) +
                                      cq[2 * plane] * test.second(1) +
                                      cq[3 * plane] * test.second(2);
                        }

                        accumulateTaggedElementalMatrix(
                            local_ke * scaling_factor, elem.id, i, j, lead.var);
                      });
          team.team_barrier();
        }
      }
    }
}

} // namespace Moose::Kokkos
