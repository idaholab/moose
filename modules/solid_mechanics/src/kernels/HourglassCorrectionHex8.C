//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HourglassCorrectionHex8.h"

#include <cmath>

registerMooseObject("SolidMechanicsApp", HourglassCorrectionHex8);

InputParameters
HourglassCorrectionHex8::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Hourglass correction for underintegrated HEX8 elements: removes the least-squares affine "
      "part of the elemental displacement and penalizes the four Flanagan-Belytschko hourglass "
      "modes with a rotation-invariant scale. 3D companion of HourglassCorrectionQuad4.");
  params.addParam<Real>("penalty", 1.0, "Base penalty parameter");
  params.addParam<Real>(
      "shear_modulus",
      1.0,
      "Shear modulus used in the hourglass stabilization scaling. Defaults to 1.0,"
      " so existing penalty-based behavior is preserved when unspecified.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HourglassCorrectionHex8::HourglassCorrectionHex8(const InputParameters & parameters)
  : Kernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _mu(getParam<Real>("shear_modulus")),
    _v(_var.dofValues()),
    // Flanagan-Belytschko patterns xi*eta, eta*zeta, xi*zeta, xi*eta*zeta in
    // libMesh HEX8 node ordering
    _gamma({{{{1, -1, 1, -1, 1, -1, 1, -1}},
             {{1, 1, -1, -1, -1, -1, 1, 1}},
             {{1, -1, -1, 1, -1, 1, 1, -1}},
             {{-1, 1, -1, 1, 1, -1, 1, -1}}}})
{
}

Real
HourglassCorrectionHex8::computeQpResidual()
{
  // Ensure single quadrature point integration and a HEX8 element.
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(_current_elem->type() == libMesh::HEX8,
              "This kernel only operates on HEX8 elements.");
  mooseAssert(_v.size() == 8, "This kernel requires 8 nodal DOF values.");

  // 1) Geometry about centroid and invariant metrics
  const Point center = _current_elem->vertex_average();
  std::array<Point, 8> dx;
  for (const auto i : make_range(8))
    dx[i] = _current_elem->node_ref(i) - center;

  // Build A = sum_i dx_i dx_i^T (symmetric 3x3)
  std::array<std::array<Real, 3>, 3> A{};
  for (const auto i : make_range(8))
    for (const auto a : make_range(3))
      for (const auto b : make_range(3))
        A[a][b] += dx[i](a) * dx[i](b);

  // Invert A robustly (adjugate / determinant with diagonal fallback)
  const Real det = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) -
                   A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) +
                   A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
  const Real eps = 1e-12;
  std::array<std::array<Real, 3>, 3> M;
  if (std::abs(det) > eps)
  {
    const Real inv = 1.0 / det;
    M[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) * inv;
    M[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) * inv;
    M[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) * inv;
    M[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) * inv;
    M[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) * inv;
    M[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) * inv;
    M[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]) * inv;
    M[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) * inv;
    M[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) * inv;
  }
  else
  {
    // Regularize: treat A as diagonal to avoid blow-up
    for (const auto a : make_range(3))
      for (const auto b : make_range(3))
        M[a][b] = 0.0;
    for (const auto a : make_range(3))
      M[a][a] = 1.0 / std::max(A[a][a], eps);
  }

  // 2) Least-squares affine fit: u_affine_i = avg + grad . dx_i
  Real avg = 0.0;
  for (const auto i : make_range(8))
    avg += _v[i];
  avg /= 8.0;

  std::array<Real, 3> b{};
  for (const auto i : make_range(8))
    for (const auto a : make_range(3))
      b[a] += _v[i] * dx[i](a);
  std::array<Real, 3> grad;
  for (const auto a : make_range(3))
    grad[a] = M[a][0] * b[0] + M[a][1] * b[1] + M[a][2] * b[2];

  std::array<Real, 8> u_hg;
  for (const auto i : make_range(8))
  {
    const Real u_aff = avg + grad[0] * dx[i](0) + grad[1] * dx[i](1) + grad[2] * dx[i](2);
    u_hg[i] = _v[i] - u_aff;
  }

  // 3) Hourglass projections onto the four Flanagan-Belytschko modes
  std::array<Real, 4> H{};
  for (const auto m : make_range(4))
    for (const auto i : make_range(8))
      H[m] += _gamma[m][i] * u_hg[i];

  // 4) Rotation-invariant scaling c = penalty * mu * V / h^2, with
  //    h^2 = trace(A)/3 and V = 8 * det(B), the one-point quadrature JxW of the
  //    trilinear map (exact volume for parallelepipeds). Using the same V as
  //    the quadrature weight keeps this kernel bitwise-consistent with the
  //    batched NEML2HourglassCorrection.
  static constexpr std::array<Real, 8> sx = {-1, 1, 1, -1, -1, 1, 1, -1};
  static constexpr std::array<Real, 8> sy = {-1, -1, 1, 1, -1, -1, 1, 1};
  static constexpr std::array<Real, 8> sz = {-1, -1, -1, -1, 1, 1, 1, 1};
  std::array<std::array<Real, 3>, 3> B{};
  for (const auto i : make_range(8))
  {
    const Point x = _current_elem->node_ref(i);
    for (const auto a : make_range(3))
    {
      B[0][a] += sx[i] * x(a) / 8.0;
      B[1][a] += sy[i] * x(a) / 8.0;
      B[2][a] += sz[i] * x(a) / 8.0;
    }
  }
  const Real detB = B[0][0] * (B[1][1] * B[2][2] - B[1][2] * B[2][1]) -
                    B[0][1] * (B[1][0] * B[2][2] - B[1][2] * B[2][0]) +
                    B[0][2] * (B[1][0] * B[2][1] - B[1][1] * B[2][0]);
  const Real vol = 8.0 * std::abs(detB);
  const Real h2 = std::max((A[0][0] + A[1][1] + A[2][2]) / 3.0, eps);
  const Real c = _penalty * _mu * (vol / h2);

  // 5) Residual contribution at node _i
  Real r = 0.0;
  for (const auto m : make_range(4))
    r += _gamma[m][_i] * H[m];
  return c * r;
}

Real
HourglassCorrectionHex8::computeQpJacobian()
{
  mooseDoOnce(mooseWarning("This kernel should only be used with explicit time integration."));
  return 0.0;
}
