//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HourglassCorrectionHex8.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

#include <algorithm>
#include <cmath>
#include <limits>

registerMooseObject("SolidMechanicsApp", HourglassCorrectionHex8);

InputParameters
HourglassCorrectionHex8::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Hourglass correction for underintegrated HEX8 elements: removes the least-squares affine "
      "part of the elemental displacement and penalizes the four Flanagan-Belytschko hourglass "
      "modes with a rotation-invariant scale. 3D companion of HourglassCorrectionQuad4.");
  params.addRangeCheckedParam<Real>(
      "penalty", 1.0, "penalty >= 0", "Dimensionless hourglass penalty coefficient");
  params.addRangeCheckedParam<Real>(
      "shear_modulus",
      1.0,
      "shear_modulus > 0",
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
  if (_qrule->n_points() != 1)
    mooseError("HourglassCorrectionHex8 requires single-point quadrature, but the current "
               "quadrature rule has ",
               _qrule->n_points(),
               " points.");
  if (_current_elem->type() != libMesh::HEX8)
    mooseError("HourglassCorrectionHex8 only supports HEX8 elements, but element ",
               _current_elem->id(),
               " has type ",
               libMesh::Utility::enum_to_string(_current_elem->type()),
               ".");
  if (_v.size() != 8)
    mooseError("HourglassCorrectionHex8 requires eight nodal degrees of freedom, but variable '",
               _var.name(),
               "' has ",
               _v.size(),
               " on element ",
               _current_elem->id(),
               ".");
  mooseAssert(_qp == 0, "Single-point quadrature must only have quadrature point zero.");

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
  const Real relative_tolerance = 1e-12;
  const Real tiny = std::numeric_limits<Real>::min();
  std::array<std::array<Real, 3>, 3> M;
  if (std::abs(det) > relative_tolerance * std::max(A[0][0] * A[1][1] * A[2][2], tiny))
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
      M[a][a] = 1.0 / std::max(A[a][a], tiny);
  }

  // 2) Project each classical hourglass vector out of the affine displacement space.
  std::array<std::array<Real, 8>, 4> projected_gamma;
  for (const auto m : make_range(4))
  {
    std::array<Real, 3> p{};
    for (const auto i : make_range(8))
      for (const auto a : make_range(3))
        p[a] += _gamma[m][i] * dx[i](a);

    std::array<Real, 3> projection;
    for (const auto a : make_range(3))
      projection[a] = M[a][0] * p[0] + M[a][1] * p[1] + M[a][2] * p[2];

    for (const auto i : make_range(8))
      projected_gamma[m][i] = _gamma[m][i] - projection[0] * dx[i](0) - projection[1] * dx[i](1) -
                              projection[2] * dx[i](2);
  }

  // 3) Hourglass amplitudes
  std::array<Real, 4> H{};
  for (const auto m : make_range(4))
    for (const auto i : make_range(8))
      H[m] += projected_gamma[m][i] * _v[i];

  // 4) Pointwise scale. Kernel assembly supplies the one-point JxWxT measure, yielding the
  // integrated coefficient penalty * mu * volume / h^2.
  const Real h2 = std::max((A[0][0] + A[1][1] + A[2][2]) / 3.0, tiny);
  const Real c = _penalty * _mu / h2;

  // 5) Residual contribution at node _i
  Real r = 0.0;
  for (const auto m : make_range(4))
    r += projected_gamma[m][_i] * H[m];
  return c * r;
}

Real
HourglassCorrectionHex8::computeQpJacobian()
{
  mooseDoOnce(mooseWarning("This kernel should only be used with explicit time integration."));
  return 0.0;
}
