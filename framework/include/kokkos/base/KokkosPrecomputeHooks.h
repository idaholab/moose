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
 * Gather the JxW-weighted coefficients of the variation of the residual of a scalar precompute
 * object posed on the current configuration along a displacement degree of freedom, the shape
 * function phi_j of displacement component c. With h = grad_x phi_j, the framework supplies the
 * variations of the configuration,
 *
 *   d(grad_x w) = -(grad_x w)_c h,   d(JxW) = JxW (h_c + phi_j (grad_x log rho)_c)
 *
 * on elements, where rho is the coordinate transform factor, and d(JxW) loses the normal part
 * n_c (n . h) of h_c on sides. The variations of the coefficients follow from the Jacobian hooks of
 * the object evaluated along d(grad_x u) and the off-diagonal hooks along d(grad_x v) for each
 * coupled field variable v, through a replacement of the trial function. The variation of the test
 * function gradient contributes -(f_grad . h) e_c to the gradient coefficient.
 */
template <typename Object>
KOKKOS_FUNCTION auto
precomputeDisplacementJacobianTerms(const Object & object,
                                    AssemblyDatum & datum,
                                    const unsigned int c,
                                    const unsigned int j,
                                    const unsigned int qp)
{
  using Field = PrecomputeField<false>;

  PrecomputedTerms<Real, Real3> terms{};

  const auto phi = VariablePhiValue()(datum, j, qp);
  const auto h = VariablePhiGradient()(datum, j, qp);
  const auto JxW = datum.JxW(qp);

  Real dmeasure = h(c) + phi * datum.grad_log_coord(qp)(c);

  if (datum.isSide())
  {
    const auto n = NormalAccessor()(datum, qp);
    dmeasure -= n(c) * (n * h);
  }

  datum.displacementVariation(true);

  Real f_value = 0, df_value = 0;
  Real3 f_grad = 0, df_grad = 0;

  if constexpr (has_precomputeQpResidualValue_v<Object>)
    f_value = object.template precomputeQpResidualValue<Object>(qp, datum);
  if constexpr (has_precomputeQpResidualGrad_v<Object>)
    f_grad = object.template precomputeQpResidualGrad<Object>(qp, datum);

  const auto sys = datum.sys();
  const auto tag = datum.tag();

  AssemblyDatum::TrialOverride trial{0, -datum.gradient(sys, datum.ivar(), tag, qp)(c) * h};
  datum.trialOverride(&trial);

  if constexpr (has_precomputeQpJacobianValue_v<Object>)
    df_value += object.template precomputeQpJacobianValue<Object>(j, qp, datum);
  if constexpr (has_precomputeQpJacobianGrad_v<Object>)
    df_grad += object.template precomputeQpJacobianGrad<Object>(j, qp, datum);

  if constexpr (has_precomputeQpOffDiagJacobianValue_v<Object> ||
                has_precomputeQpOffDiagJacobianGrad_v<Object>)
  {
    const auto & system = datum.system(sys);
    const auto & coupling = system.getFieldCoupling(datum.ivar());

    for (unsigned int k = 0; k < coupling.size(); ++k)
    {
      const auto v = coupling[k];

      if (!system.isVariableActive(v, datum.subdomain()))
        continue;

      trial.grad_phi = -datum.gradient(sys, v, tag, qp)(c) * h;

      if constexpr (has_precomputeQpOffDiagJacobianValue_v<Object>)
        df_value += object.template precomputeQpOffDiagJacobianValue<Object>(j, v, qp, datum);
      if constexpr (has_precomputeQpOffDiagJacobianGrad_v<Object>)
        df_grad += object.template precomputeQpOffDiagJacobianGrad<Object>(j, v, qp, datum);
    }
  }

  datum.trialOverride(nullptr);
  datum.displacementVariation(false);

  Real3 e_c(0);
  e_c(c) = 1;

  terms.value = JxW * (dmeasure * f_value + df_value);
  terms.grad = Field::pullBack(JxW * (dmeasure * f_grad + df_grad - (f_grad * h) * e_c), datum, qp);

  return terms;
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
  // Displacement component discretized by the column variable of an object posed on the current
  // configuration
  const auto c = datum.configuration().component(datum.sys(), datum.jvar());

  return [&, c](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
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

      if constexpr (!is_vector)
        if (c != libMesh::invalid_uint)
        {
          const auto displacement_terms =
              precomputeDisplacementJacobianTerms(object, datum, c, j, qp);

          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] +=
                contractTerms<false, true, true, false>(displacement_terms, datum, i, qp);
        }
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
