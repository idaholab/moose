//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosPrecomputeHookTraits.h"
#include "KokkosVariableValue.h"
#include "KokkosDispatcher.h"

namespace Moose::Kokkos
{

/**
 * The per-quadrature-point coefficients of the precompute hooks of one assembled quantity,
 * transformed for contraction with the test functions
 */
template <typename Coefficient, typename GradCoefficient>
struct PrecomputedTerms
{
  /// Coefficient of the test function value
  Coefficient value;
  /// Gradient coefficient pulled back to the reference element, for contraction with the
  /// reference test function gradient
  GradCoefficient grad;
  /// Coefficient of the test function curl
  Coefficient curl;
};

/**
 * Test function quantities of the field types assembled by precompute residual objects
 */
///@{
template <bool is_vector>
struct PrecomputeField;

template <>
struct PrecomputeField<false>
{
  using Coefficient = Real;
  using GradCoefficient = Real3;

  /**
   * Pull a gradient coefficient back to the reference element, so that its product with the
   * reference test function gradient equals its product with the physical one
   */
  KOKKOS_FUNCTION static Real3 pullBack(const Real3 & grad, AssemblyDatum & datum, unsigned int qp)
  {
    return datum.dxi_dx(qp).transpose() * grad;
  }
  KOKKOS_FUNCTION static Real
  value(const Real coef, AssemblyDatum & datum, unsigned int i, unsigned int qp)
  {
    return coef * VariableTestValue()(datum, i, qp);
  }
  KOKKOS_FUNCTION static Real
  grad(const Real3 & coef, AssemblyDatum & datum, unsigned int i, unsigned int qp)
  {
    return coef * VariableTestGradient().reference(datum, i, qp);
  }
};

template <>
struct PrecomputeField<true>
{
  using Coefficient = Real3;
  using GradCoefficient = Real33;

  KOKKOS_FUNCTION static Real33
  pullBack(const Real33 & grad, AssemblyDatum & datum, unsigned int qp)
  {
    return grad * datum.dxi_dx(qp);
  }
  KOKKOS_FUNCTION static Real
  value(const Real3 & coef, AssemblyDatum & datum, unsigned int i, unsigned int qp)
  {
    return coef * VectorVariableTestValue()(datum, i, qp);
  }
  KOKKOS_FUNCTION static Real
  grad(const Real33 & coef, AssemblyDatum & datum, unsigned int i, unsigned int qp)
  {
    return coef.contract(VectorVariableTestGradient().reference(datum, i, qp));
  }
  KOKKOS_FUNCTION static Real
  curl(const Real3 & coef, AssemblyDatum & datum, unsigned int i, unsigned int qp)
  {
    return coef * VectorVariableTestCurl()(datum, i, qp);
  }
};
///@}

/**
 * Gather the JxW-weighted coefficients of one assembled quantity at a quadrature point. Each of
 * the callables value, grad, and curl invokes the corresponding hook and is called only when the
 * object defines that hook.
 */
template <bool is_vector,
          bool has_value,
          bool has_grad,
          bool has_curl,
          typename Value,
          typename Grad,
          typename Curl>
KOKKOS_FUNCTION auto
precomputeTerms(AssemblyDatum & datum, const unsigned int qp, Value value, Grad grad, Curl curl)
{
  using Field = PrecomputeField<is_vector>;

  PrecomputedTerms<typename Field::Coefficient, typename Field::GradCoefficient> terms{};

  const auto JxW = datum.JxW(qp);

  if constexpr (has_value)
    terms.value = JxW * value();
  if constexpr (has_grad)
    terms.grad = Field::pullBack(JxW * grad(), datum, qp);
  if constexpr (has_curl)
    terms.curl = JxW * curl();

  return terms;
}

/**
 * Contract precomputed coefficients with the test functions of one test function DOF
 */
template <bool is_vector, bool has_value, bool has_grad, bool has_curl, typename Terms>
KOKKOS_FUNCTION Real
contractTerms(const Terms & terms,
              AssemblyDatum & datum,
              const unsigned int i,
              const unsigned int qp)
{
  using Field = PrecomputeField<is_vector>;

  Real result = 0;

  if constexpr (has_value)
    result += Field::value(terms.value, datum, i, qp);
  if constexpr (has_grad)
    result += Field::grad(terms.grad, datum, i, qp);
  if constexpr (has_curl)
    result += Field::curl(terms.curl, datum, i, qp);

  return result;
}

/**
 * Hook invocations of each assembled quantity, gathered into precomputed terms
 */
///@{
template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeResidualTerms(const Object & object, AssemblyDatum & datum, const unsigned int qp)
{
  return precomputeTerms<is_vector,
                         has_precomputeQpResidualValue_v<Object>,
                         has_precomputeQpResidualGrad_v<Object>,
                         has_precomputeQpResidualCurl_v<Object>>(
      datum,
      qp,
      [&]()
      {
        if constexpr (has_precomputeQpResidualValue_v<Object>)
          return object.template precomputeQpResidualValue<Object>(qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpResidualGrad_v<Object>)
          return object.template precomputeQpResidualGrad<Object>(qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpResidualCurl_v<Object>)
          return object.template precomputeQpResidualCurl<Object>(qp, datum);
      });
}

template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeJacobianTerms(const Object & object,
                        AssemblyDatum & datum,
                        const unsigned int j,
                        const unsigned int qp)
{
  return precomputeTerms<is_vector,
                         has_precomputeQpJacobianValue_v<Object>,
                         has_precomputeQpJacobianGrad_v<Object>,
                         has_precomputeQpJacobianCurl_v<Object>>(
      datum,
      qp,
      [&]()
      {
        if constexpr (has_precomputeQpJacobianValue_v<Object>)
          return object.template precomputeQpJacobianValue<Object>(j, qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpJacobianGrad_v<Object>)
          return object.template precomputeQpJacobianGrad<Object>(j, qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpJacobianCurl_v<Object>)
          return object.template precomputeQpJacobianCurl<Object>(j, qp, datum);
      });
}
template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeOffDiagJacobianTerms(const Object & object,
                               AssemblyDatum & datum,
                               const unsigned int j,
                               const unsigned int qp)
{
  const auto jvar = datum.jvar();

  return precomputeTerms<is_vector,
                         has_precomputeQpOffDiagJacobianValue_v<Object>,
                         has_precomputeQpOffDiagJacobianGrad_v<Object>,
                         has_precomputeQpOffDiagJacobianCurl_v<Object>>(
      datum,
      qp,
      [&]()
      {
        if constexpr (has_precomputeQpOffDiagJacobianValue_v<Object>)
          return object.template precomputeQpOffDiagJacobianValue<Object>(j, jvar, qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpOffDiagJacobianGrad_v<Object>)
          return object.template precomputeQpOffDiagJacobianGrad<Object>(j, jvar, qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpOffDiagJacobianCurl_v<Object>)
          return object.template precomputeQpOffDiagJacobianCurl<Object>(j, jvar, qp, datum);
      });
}

template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeOffDiagJacobianScalarTerms(const Object & object,
                                     AssemblyDatum & datum,
                                     const unsigned int j,
                                     const unsigned int qp)
{
  const auto jvar = datum.jvar();

  return precomputeTerms<is_vector,
                         has_precomputeQpOffDiagJacobianScalarValue_v<Object>,
                         has_precomputeQpOffDiagJacobianScalarGrad_v<Object>,
                         has_precomputeQpOffDiagJacobianScalarCurl_v<Object>>(
      datum,
      qp,
      [&]()
      {
        if constexpr (has_precomputeQpOffDiagJacobianScalarValue_v<Object>)
          return object.template precomputeQpOffDiagJacobianScalarValue<Object>(j, jvar, qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpOffDiagJacobianScalarGrad_v<Object>)
          return object.template precomputeQpOffDiagJacobianScalarGrad<Object>(j, jvar, qp, datum);
      },
      [&]()
      {
        if constexpr (has_precomputeQpOffDiagJacobianScalarCurl_v<Object>)
          return object.template precomputeQpOffDiagJacobianScalarCurl<Object>(j, jvar, qp, datum);
      });
}
///@}

/**
 * Variation -(grad_x w) e_c (x) h of the gradient of a field on the configuration along a
 * displacement of component c with shape function gradient h
 */
///@{
KOKKOS_INLINE_FUNCTION Real3
displacementGradientVariation(const Real3 & grad, const unsigned int c, const Real3 & h)
{
  return -grad(c) * h;
}
KOKKOS_INLINE_FUNCTION Real33
displacementGradientVariation(const Real33 & grad, const unsigned int c, const Real3 & h)
{
  Real33 variation;

  for (unsigned int a = 0; a < 3; ++a)
    for (unsigned int b = 0; b < 3; ++b)
      variation(a, b) = -grad(a, c) * h(b);

  return variation;
}
///@}

/**
 * Get the tensor C with C : A = f . curl(A) for the curl of a vector field formed from its gradient
 * A, so that a curl coefficient f can be contracted with a gradient
 * @param f The curl coefficient
 * @param dim The mesh dimension
 */
KOKKOS_INLINE_FUNCTION Real33
curlCoefficientAsGradient(const Real3 & f, const unsigned int dim)
{
  Real33 C;

  if (dim == 2)
  {
    C(1, 0) = f(2);
    C(0, 1) = -f(2);
  }
  else if (dim == 3)
  {
    C(2, 1) = f(0);
    C(1, 2) = -f(0);
    C(0, 2) = f(1);
    C(2, 0) = -f(1);
    C(1, 0) = f(2);
    C(0, 1) = -f(2);
  }

  return C;
}

/**
 * Gather the JxW-weighted coefficients of the variation of the residual of a precompute object
 * posed on the current configuration along a displacement degree of freedom, the shape function
 * phi_j of displacement component c. With h = grad_x phi_j, the framework supplies the variations
 * of the configuration,
 *
 *   d(grad_x w) = -(grad_x w) e_c (x) h,   d(JxW) = JxW (h_c + phi_j (grad_x log rho)_c)
 *
 * on elements, where rho is the coordinate transform factor, and d(JxW) loses the normal part
 * n_c (n . h) of h_c on sides. Values do not vary. The variations of the coefficients follow from
 * the Jacobian hooks of the object evaluated along d(grad_x u) and the off-diagonal hooks along
 * d(grad_x v) for each coupled field variable v, through a replacement of the trial function. The
 * variation of the test function gradient contributes -(f_grad h) (x) e_c to the gradient
 * coefficient, and that of the test function curl, which is formed from the gradient, contributes
 * through the tensor C of the curl coefficient with C : grad = f_curl . curl. The quadrature point
 * x = X + u moves by dx = phi_j e_c, and on sides the normal n = F^{-T} N / |F^{-T} N| varies by
 * dn = -n_c (h - (n . h) n); the position and normal Jacobian hooks of the object contract these
 * with the derivatives of its coefficients.
 *
 * The variation is linear in (phi_j, h), because the Jacobian, position, and normal Jacobian hooks
 * are linear in their trial function and variation arguments. The assembly therefore evaluates it
 * once per quadrature point along four directions and combines the results for each column (see
 * assembleDisplacementJacobian()).
 * @param phi The value of the displacement shape function
 * @param h The gradient of the displacement shape function on the configuration
 */
template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeDisplacementJacobianTerms(const Object & object,
                                    AssemblyDatum & datum,
                                    const unsigned int c,
                                    const Real phi,
                                    const Real3 & h,
                                    const unsigned int qp)
{
  using Field = PrecomputeField<is_vector>;
  using Coefficient = typename Field::Coefficient;
  using GradCoefficient = typename Field::GradCoefficient;

  PrecomputedTerms<Coefficient, GradCoefficient> terms{};

  // The hooks receive a placeholder column index; the trial function they read is replaced
  const unsigned int j = 0;
  const auto JxW = datum.JxW(qp);

  Real dmeasure = h(c) + phi * datum.grad_log_coord(qp)(c);
  Real3 dn = 0;

  if (datum.isSide())
  {
    const auto n = NormalAccessor()(datum, qp);
    dmeasure -= n(c) * (n * h);
    dn = -n(c) * (h - (n * h) * n);
  }

  Real3 dx = 0;
  dx(c) = phi;

  datum.displacementVariation(
      true, has_normal_jacobian_hook<Object>, has_position_jacobian_hook<Object>);

  Coefficient f_value = 0, df_value = 0, f_curl = 0, df_curl = 0;
  GradCoefficient f_grad = 0, df_grad = 0;

  if constexpr (has_precomputeQpResidualValue_v<Object>)
    f_value = object.template precomputeQpResidualValue<Object>(qp, datum);
  if constexpr (has_precomputeQpResidualGrad_v<Object>)
    f_grad = object.template precomputeQpResidualGrad<Object>(qp, datum);
  if constexpr (has_precomputeQpResidualCurl_v<Object>)
    f_curl = object.template precomputeQpResidualCurl<Object>(qp, datum);

  if constexpr (has_precomputeQpPositionJacobianValue_v<Object>)
    df_value += object.template precomputeQpPositionJacobianValue<Object>(dx, qp, datum);
  if constexpr (has_precomputeQpPositionJacobianGrad_v<Object>)
    df_grad += object.template precomputeQpPositionJacobianGrad<Object>(dx, qp, datum);
  if constexpr (has_precomputeQpPositionJacobianCurl_v<Object>)
    df_curl += object.template precomputeQpPositionJacobianCurl<Object>(dx, qp, datum);

  if (datum.isSide())
  {
    if constexpr (has_precomputeQpNormalJacobianValue_v<Object>)
      df_value += object.template precomputeQpNormalJacobianValue<Object>(dn, qp, datum);
    if constexpr (has_precomputeQpNormalJacobianGrad_v<Object>)
      df_grad += object.template precomputeQpNormalJacobianGrad<Object>(dn, qp, datum);
    if constexpr (has_precomputeQpNormalJacobianCurl_v<Object>)
      df_curl += object.template precomputeQpNormalJacobianCurl<Object>(dn, qp, datum);
  }

  const auto sys = datum.sys();
  const auto tag = datum.tag();
  const auto & system = datum.system(sys);
  const auto & assembly = datum.assembly();

  // Replace the trial function of a variable by the variation of its gradient
  AssemblyDatum::TrialOverride trial{0, Real3(0), Real3(0), Real33(0)};
  auto vary = [&](const unsigned int var)
  {
    trial = {0, Real3(0), Real3(0), Real33(0)};

    if (assembly.isVectorFEType(system.getFETypeID(var)))
      trial.vector_grad_phi =
          displacementGradientVariation(datum.vectorGradient(sys, var, tag, qp), c, h);
    else
      trial.grad_phi = displacementGradientVariation(datum.gradient(sys, var, tag, qp), c, h);
  };

  // The variation of a field gradient vanishes along the direction of the shape function value
  const bool varies_gradients = h(0) != 0 || h(1) != 0 || h(2) != 0;

  if constexpr (has_jacobian_hook<Object>)
    if (varies_gradients)
      vary(datum.ivar());

  datum.trialOverride(&trial);

  if constexpr (has_precomputeQpJacobianValue_v<Object>)
    df_value += object.template precomputeQpJacobianValue<Object>(j, qp, datum);
  if constexpr (has_precomputeQpJacobianGrad_v<Object>)
    df_grad += object.template precomputeQpJacobianGrad<Object>(j, qp, datum);
  if constexpr (has_precomputeQpJacobianCurl_v<Object>)
    df_curl += object.template precomputeQpJacobianCurl<Object>(j, qp, datum);

  if constexpr (has_offdiag_jacobian_hook<Object>)
    if (varies_gradients)
    {
      const auto & coupling = system.getFieldCoupling(datum.ivar());

      for (unsigned int k = 0; k < coupling.size(); ++k)
      {
        const auto v = coupling[k];

        if (!system.isVariableActive(v, datum.subdomain()))
          continue;

        vary(v);

        if constexpr (has_precomputeQpOffDiagJacobianValue_v<Object>)
          df_value += object.template precomputeQpOffDiagJacobianValue<Object>(j, v, qp, datum);
        if constexpr (has_precomputeQpOffDiagJacobianGrad_v<Object>)
          df_grad += object.template precomputeQpOffDiagJacobianGrad<Object>(j, v, qp, datum);
        if constexpr (has_precomputeQpOffDiagJacobianCurl_v<Object>)
          df_curl += object.template precomputeQpOffDiagJacobianCurl<Object>(j, v, qp, datum);
      }
    }

  datum.trialOverride(nullptr);
  datum.displacementVariation(false, false, false);

  // Variation of the test function gradient, and of the test function curl expressed through the
  // gradient
  GradCoefficient test_variation = 0;

  if constexpr (!is_vector)
    test_variation(c) = -(f_grad * h);
  else
  {
    const auto f_h = f_grad * h + curlCoefficientAsGradient(f_curl, assembly.getDimension()) * h;

    for (unsigned int a = 0; a < 3; ++a)
      test_variation(a, c) = -f_h(a);
  }

  terms.value = JxW * (dmeasure * f_value + df_value);

  GradCoefficient grad = dmeasure * f_grad;
  grad += df_grad;
  grad += test_variation;
  terms.grad = Field::pullBack(JxW * grad, datum, qp);

  if constexpr (is_vector)
    terms.curl = JxW * (dmeasure * f_curl + df_curl);

  return terms;
}

/**
 * Layout of the scratch memory of one group of the displacement Jacobian of a precompute object.
 * For a chunk of quadrature points, the scratch holds at [qp][d][i] the coefficients of the
 * variation along direction d contracted with test function i. The directions are (phi, h) =
 * (1, 0) and (0, M e_b) for b = 0, 1, 2, where M maps reference gradients to the configuration.
 */
struct DisplacementScratchLayout
{
  /// Number of directions of the variation
  static constexpr unsigned int num_directions = 4;
  /// Whether the groups align to half-warps, so that the scratch accesses of two groups never share
  /// a shared memory wavefront
  static constexpr bool align_groups = true;

  /**
   * Get the number of reals stored per quadrature point. The count is odd, so that the stores of
   * the quadrature points of a group fall in distinct shared memory banks.
   */
  KOKKOS_FUNCTION unsigned int qpSize() const { return num_directions * max_idofs + 1; }
  /**
   * Get the number of reals of the scratch of a group
   */
  KOKKOS_FUNCTION unsigned int groupSize() const { return qp_chunk * qpSize(); }
  /**
   * Get the number of bytes of scratch memory per team
   * @param groups The number of groups of the team, each assembling one element or side
   */
  std::size_t bytes(const unsigned int groups) const
  {
    return ::Kokkos::View<Real *, ::Kokkos::DefaultExecutionSpace::scratch_memory_space>::
        shmem_size(groups * groupSize());
  }

  /// Maximum number of quadrature points of an element or side
  unsigned int max_qps = 0;
  /// Maximum number of test function DOFs of an element or side
  unsigned int max_idofs = 0;
  /// Number of quadrature points per chunk
  unsigned int qp_chunk = 0;
};

/**
 * Assemble the Jacobian of a precompute object posed on the current configuration with respect to
 * one displacement component on the element or side of one group of a team. With h = M ref_j the
 * gradient of the displacement shape function on the configuration, the linearity of the variation
 * T(phi_j, h) of precomputeDisplacementJacobianTerms() gives
 *
 *   T(phi_j, h) = phi_j T(1, 0) + sum_b ref_jb T(0, M e_b)
 *
 * Stage 1 evaluates the four directions once per quadrature point and contracts each with the test
 * functions. Stage 2 combines them for each entry (i, j) with the reference shape function and
 * gradient of column j:
 *
 *   J_ij = sum_qp (phi_j D_0i + sum_b ref_jb D_(1 + b)i)
 *
 * @param object The residual object of the final derived type
 * @param team The team handle
 * @param group The group of the thread
 * @param layout The scratch layout of the dispatch
 * @param active Whether the group assembles its element or side. A group that does not still
 * reaches the team barriers.
 * @param datum The AssemblyDatum object of the thread, whose column variable is the displacement
 * component
 * @param c The displacement component
 * @param accumulate The callable accumulating an entry (i, j) of the local matrix
 */
template <bool is_vector, typename Object, typename Team, typename Accumulate>
KOKKOS_FUNCTION void
assembleDisplacementJacobian(const Object & object,
                             const Team & team,
                             const TeamGroup & group,
                             const DisplacementScratchLayout & layout,
                             const bool active,
                             AssemblyDatum & datum,
                             const unsigned int c,
                             Accumulate accumulate)
{
  constexpr auto num_directions = DisplacementScratchLayout::num_directions;

  const auto group_scratch_size = layout.groupSize();
  ::Kokkos::
      View<Real *, ExecSpace::scratch_memory_space, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>>
          team_scratch(team.team_scratch(0), group.count() * group_scratch_size);
  Real * const scratch = &team_scratch(group.index() * group_scratch_size);

  const auto & assembly = datum.assembly();
  const auto & elem = datum.elem();
  const auto side = datum.side();
  const auto jfe = datum.jfe();
  const auto n_qps = datum.n_qps();
  const auto n_idofs = datum.n_idofs();
  const auto n_jdofs = datum.n_jdofs();
  const auto qp_size = layout.qpSize();
  const auto stride = layout.max_idofs;
  const auto chunk = layout.qp_chunk;
  KOKKOS_ASSERT(chunk > 0);
  // Every group runs the same number of chunks, so that the team barriers match
  const auto n_chunks = (layout.max_qps + chunk - 1) / chunk;

  for (unsigned int ch = 0; ch < n_chunks; ++ch)
  {
    const auto qp_begin = ch * chunk;
    const auto n_chunk_qps = qp_begin < n_qps ? ::Kokkos::min(chunk, n_qps - qp_begin) : 0u;

    // Stage 1: each thread evaluates its quadrature points with its own datum, whose geometry cache
    // and trial function replacement are thread-private
    if (active)
      for (unsigned int q = group.rank(); q < n_chunk_qps; q += group.size())
      {
        const auto qp = qp_begin + q;
        Real * const D = scratch + q * qp_size;

        for (unsigned int d = 0; d < num_directions; ++d)
        {
          Real3 h(0);
          if (d)
          {
            const auto & M = datum.dxi_dx(qp);
            for (unsigned int a = 0; a < 3; ++a)
              h(a) = M(a, d - 1);
          }

          const auto terms =
              precomputeDisplacementJacobianTerms<is_vector>(object, datum, c, Real(d == 0), h, qp);

          for (unsigned int i = 0; i < n_idofs; ++i)
            D[d * stride + i] =
                contractTerms<is_vector, true, true, is_vector>(terms, datum, i, qp);
        }
      }
    team.team_barrier();

    // Stage 2: combine the directions for each entry
    if (active && n_chunk_qps)
      for (unsigned int k = group.rank(); k < n_idofs * n_jdofs; k += group.size())
      {
        const auto i = k / n_jdofs;
        const auto j = k % n_jdofs;
        Real local_ke = 0;

        for (unsigned int q = 0; q < n_chunk_qps; ++q)
        {
          const auto qp = qp_begin + q;
          const Real phi =
              side == libMesh::invalid_uint
                  ? assembly.getPhi(elem.subdomain, elem.type, jfe)(j, qp)
                  : assembly.getPhiFace(elem.subdomain, elem.type, jfe)(side)(j, qp);
          const Real3 ref =
              side == libMesh::invalid_uint
                  ? assembly.getGradPhi(elem.subdomain, elem.type, jfe)(j, qp)
                  : assembly.getGradPhiFace(elem.subdomain, elem.type, jfe)(side)(j, qp);
          const Real * const D = scratch + q * qp_size + i;

          local_ke += phi * D[0] + ref(0) * D[stride] + ref(1) * D[2 * stride] +
                      ref(2) * D[3 * stride];
        }

        accumulate(local_ke, i, j);
      }

    // The next chunk overwrites the scratch
    if (ch + 1 < n_chunks)
      team.team_barrier();
  }
}

/**
 * Assembly loop bodies of precompute residual objects, passed to
 * ResidualObject::computeResidualInternal() and ResidualObject::computeJacobianInternal()
 */
///@{
template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeResidualBody(const Object & object, AssemblyDatum & datum)
{
  return [&](Real * local_re, const unsigned int ib, const unsigned int ie)
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      const auto terms = precomputeResidualTerms<is_vector>(object, datum, qp);

      for (unsigned int i = ib; i < ie; ++i)
        local_re[i] += contractTerms<is_vector,
                                     has_precomputeQpResidualValue_v<Object>,
                                     has_precomputeQpResidualGrad_v<Object>,
                                     has_precomputeQpResidualCurl_v<Object>>(terms, datum, i, qp);
    }
  };
}

template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeJacobianBody(const Object & object, AssemblyDatum & datum)
{
  return [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      const auto terms = precomputeJacobianTerms<is_vector>(object, datum, j, qp);

      for (unsigned int i = ib; i < ie; ++i)
        local_ke[i] += contractTerms<is_vector,
                                     has_precomputeQpJacobianValue_v<Object>,
                                     has_precomputeQpJacobianGrad_v<Object>,
                                     has_precomputeQpJacobianCurl_v<Object>>(terms, datum, i, qp);
    }
  };
}

template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeOffDiagJacobianBody(const Object & object, AssemblyDatum & datum)
{
  return [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      const auto terms = precomputeOffDiagJacobianTerms<is_vector>(object, datum, j, qp);

      for (unsigned int i = ib; i < ie; ++i)
        local_ke[i] +=
            contractTerms<is_vector,
                          has_precomputeQpOffDiagJacobianValue_v<Object>,
                          has_precomputeQpOffDiagJacobianGrad_v<Object>,
                          has_precomputeQpOffDiagJacobianCurl_v<Object>>(terms, datum, i, qp);
    }
  };
}

template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeOffDiagJacobianScalarBody(const Object & object, AssemblyDatum & datum)
{
  return [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      const auto terms = precomputeOffDiagJacobianScalarTerms<is_vector>(object, datum, j, qp);

      for (unsigned int i = ib; i < ie; ++i)
        local_ke[i] +=
            contractTerms<is_vector,
                          has_precomputeQpOffDiagJacobianScalarValue_v<Object>,
                          has_precomputeQpOffDiagJacobianScalarGrad_v<Object>,
                          has_precomputeQpOffDiagJacobianScalarCurl_v<Object>>(terms, datum, i, qp);
    }
  };
}
///@}

} // namespace Moose::Kokkos
