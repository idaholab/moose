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
 */
template <bool is_vector, typename Object>
KOKKOS_FUNCTION auto
precomputeDisplacementJacobianTerms(const Object & object,
                                    AssemblyDatum & datum,
                                    const unsigned int c,
                                    const unsigned int j,
                                    const unsigned int qp)
{
  using Field = PrecomputeField<is_vector>;
  using Coefficient = typename Field::Coefficient;
  using GradCoefficient = typename Field::GradCoefficient;

  PrecomputedTerms<Coefficient, GradCoefficient> terms{};

  const auto phi = VariablePhiValue()(datum, j, qp);
  const auto h = VariablePhiGradient()(datum, j, qp);
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

  vary(datum.ivar());
  datum.trialOverride(&trial);

  if constexpr (has_precomputeQpJacobianValue_v<Object>)
    df_value += object.template precomputeQpJacobianValue<Object>(j, qp, datum);
  if constexpr (has_precomputeQpJacobianGrad_v<Object>)
    df_grad += object.template precomputeQpJacobianGrad<Object>(j, qp, datum);
  if constexpr (has_precomputeQpJacobianCurl_v<Object>)
    df_curl += object.template precomputeQpJacobianCurl<Object>(j, qp, datum);

  if constexpr (has_offdiag_jacobian_hook<Object>)
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

      if (c != libMesh::invalid_uint)
      {
        const auto displacement_terms =
            precomputeDisplacementJacobianTerms<is_vector>(object, datum, c, j, qp);

        for (unsigned int i = ib; i < ie; ++i)
          local_ke[i] +=
              contractTerms<is_vector, true, true, is_vector>(displacement_terms, datum, i, qp);
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
