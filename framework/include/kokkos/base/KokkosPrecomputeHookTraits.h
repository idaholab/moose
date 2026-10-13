//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

#include <type_traits>
#include <utility>

namespace Moose::Kokkos
{

class AssemblyDatum;

/**
 * Detection of the precompute hooks of a residual object. A residual object factors its weak form
 * at each quadrature point as a sum of terms, each the product of a coefficient computed by a hook
 * and one test function quantity:
 *
 *   R_i = sum_qp JxW (f_value . psi_i + f_grad : grad psi_i + f_curl . curl psi_i)
 *
 * The hook names carry the test function quantity, and each family has one hook per assembled
 * quantity:
 *
 *   precomputeQpResidual{Value,Grad,Curl}(qp, datum)
 *   precomputeQpJacobian{Value,Grad,Curl}(j, qp, datum)
 *   precomputeQpOffDiagJacobian{Value,Grad,Curl}(j, jvar, qp, datum)
 *   precomputeQpOffDiagJacobianScalar{Value,Grad,Curl}(j, jvar, qp, datum)
 *   precomputeQpNormalJacobian{Value,Grad,Curl}(dn, qp, datum)
 *   precomputeQpPositionJacobian{Value,Grad,Curl}(dx, qp, datum)
 *
 * The normal and position Jacobian hooks return the derivative of the residual coefficients along
 * a variation dn of the normal of a side or dx of the quadrature point position. The framework
 * supplies dn and dx when it assembles the Jacobian of an object posed on the current configuration
 * with respect to the displacements, which move the normals and the quadrature points; an object
 * whose residual reads the normals or the quadrature point coordinates must define the matching
 * hooks to be used there.
 *
 * The assembly loops evaluate only the terms whose hooks the object defines, so an object pays
 * nothing for the test function quantities it does not use.
 */
///@{
#define MOOSE_KOKKOS_DETECT_HOOK(hook, ...)                                                        \
  template <typename Object, typename = void>                                                      \
  struct has_##hook : std::false_type                                                              \
  {                                                                                                \
  };                                                                                               \
  template <typename Object>                                                                       \
  struct has_##hook<                                                                               \
      Object,                                                                                      \
      std::void_t<decltype(std::declval<const Object &>().template hook<Object>(__VA_ARGS__))>>    \
    : std::true_type                                                                               \
  {                                                                                                \
  };                                                                                               \
  template <typename Object>                                                                       \
  inline constexpr bool has_##hook##_v = has_##hook<Object>::value

#define MOOSE_KOKKOS_DETECT_HOOK_FAMILY(family, ...)                                               \
  MOOSE_KOKKOS_DETECT_HOOK(family##Value, __VA_ARGS__);                                            \
  MOOSE_KOKKOS_DETECT_HOOK(family##Grad, __VA_ARGS__);                                             \
  MOOSE_KOKKOS_DETECT_HOOK(family##Curl, __VA_ARGS__)

MOOSE_KOKKOS_DETECT_HOOK_FAMILY(precomputeQpResidual, 0u, std::declval<AssemblyDatum &>());
MOOSE_KOKKOS_DETECT_HOOK_FAMILY(precomputeQpJacobian, 0u, 0u, std::declval<AssemblyDatum &>());
MOOSE_KOKKOS_DETECT_HOOK_FAMILY(
    precomputeQpOffDiagJacobian, 0u, 0u, 0u, std::declval<AssemblyDatum &>());
MOOSE_KOKKOS_DETECT_HOOK_FAMILY(
    precomputeQpOffDiagJacobianScalar, 0u, 0u, 0u, std::declval<AssemblyDatum &>());
MOOSE_KOKKOS_DETECT_HOOK_FAMILY(precomputeQpNormalJacobian,
                                std::declval<const Real3 &>(),
                                0u,
                                std::declval<AssemblyDatum &>());
MOOSE_KOKKOS_DETECT_HOOK_FAMILY(precomputeQpPositionJacobian,
                                std::declval<const Real3 &>(),
                                0u,
                                std::declval<AssemblyDatum &>());

#undef MOOSE_KOKKOS_DETECT_HOOK_FAMILY
#undef MOOSE_KOKKOS_DETECT_HOOK

template <typename Object>
inline constexpr bool has_residual_hook =
    has_precomputeQpResidualValue_v<Object> || has_precomputeQpResidualGrad_v<Object> ||
    has_precomputeQpResidualCurl_v<Object>;
template <typename Object>
inline constexpr bool has_jacobian_hook =
    has_precomputeQpJacobianValue_v<Object> || has_precomputeQpJacobianGrad_v<Object> ||
    has_precomputeQpJacobianCurl_v<Object>;
template <typename Object>
inline constexpr bool has_offdiag_jacobian_hook =
    has_precomputeQpOffDiagJacobianValue_v<Object> ||
    has_precomputeQpOffDiagJacobianGrad_v<Object> || has_precomputeQpOffDiagJacobianCurl_v<Object>;
template <typename Object>
inline constexpr bool has_normal_jacobian_hook =
    has_precomputeQpNormalJacobianValue_v<Object> || has_precomputeQpNormalJacobianGrad_v<Object> ||
    has_precomputeQpNormalJacobianCurl_v<Object>;
template <typename Object>
inline constexpr bool has_position_jacobian_hook =
    has_precomputeQpPositionJacobianValue_v<Object> ||
    has_precomputeQpPositionJacobianGrad_v<Object> ||
    has_precomputeQpPositionJacobianCurl_v<Object>;
template <typename Object>
inline constexpr bool has_offdiag_jacobian_scalar_hook =
    has_precomputeQpOffDiagJacobianScalarValue_v<Object> ||
    has_precomputeQpOffDiagJacobianScalarGrad_v<Object> ||
    has_precomputeQpOffDiagJacobianScalarCurl_v<Object>;
///@}

} // namespace Moose::Kokkos
