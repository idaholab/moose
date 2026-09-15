//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HourglassCorrectionQuad4.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

#include <algorithm>
#include <cmath>
#include <limits>

registerMooseObject("SolidMechanicsApp", HourglassCorrectionQuad4);

InputParameters
HourglassCorrectionQuad4::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Hourglass stabilization for underintegrated QUAD4 elements: removes the least-squares "
      "affine part of the elemental solution, penalizes the remaining hourglass mode, and "
      "scales the penalty with a rotation-invariant measure of the current element geometry.");
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

HourglassCorrectionQuad4::HourglassCorrectionQuad4(const InputParameters & parameters)
  : Kernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _mu(getParam<Real>("shear_modulus")),
    _v(_var.dofValues()),
    _gamma({1.0, -1.0, 1.0, -1.0})
{
}

Real
HourglassCorrectionQuad4::computeQpResidual()
{
  if (_qrule->n_points() != 1)
    mooseError("HourglassCorrectionQuad4 requires single-point quadrature, but the current "
               "quadrature rule has ",
               _qrule->n_points(),
               " points.");
  if (_current_elem->type() != libMesh::QUAD4)
    mooseError("HourglassCorrectionQuad4 only supports QUAD4 elements, but element ",
               _current_elem->id(),
               " has type ",
               libMesh::Utility::enum_to_string(_current_elem->type()),
               ".");
  if (_v.size() != 4)
    mooseError("HourglassCorrectionQuad4 requires four nodal degrees of freedom, but variable '",
               _var.name(),
               "' has ",
               _v.size(),
               " on element ",
               _current_elem->id(),
               ".");
  mooseAssert(_qp == 0, "Single-point quadrature must only have quadrature point zero.");

  // 1) Geometry about centroid and invariant metrics
  const Point center = _current_elem->vertex_average();
  std::array<Point, 4> dx;
  for (const auto i : make_range(4))
    dx[i] = _current_elem->node_ref(i) - center;

  // Build A = sum_i dx_i dx_i^T
  Real A00 = 0.0, A01 = 0.0, A11 = 0.0;
  for (const auto i : make_range(4))
  {
    const Real x = dx[i](0);
    const Real y = dx[i](1);
    A00 += x * x;
    A01 += x * y;
    A11 += y * y;
  }

  // Invert A robustly
  const Real det = A00 * A11 - A01 * A01;
  const Real relative_tolerance = 1e-12;
  const Real tiny = std::numeric_limits<Real>::min();
  Real M00, M01, M10, M11;
  if (std::abs(det) > relative_tolerance * std::max(A00 * A11, tiny))
  {
    const Real inv = 1.0 / det;
    M00 = A11 * inv;
    M01 = -A01 * inv;
    M10 = -A01 * inv;
    M11 = A00 * inv;
  }
  else
  {
    // Regularize: treat A as diagonal with small size to avoid blow-up
    const Real reg = std::max(A00 + A11, tiny);
    M00 = 1.0 / std::max(A00, reg);
    M01 = 0.0;
    M10 = 0.0;
    M11 = 1.0 / std::max(A11, reg);
  }

  // 2) Project the classical hourglass vector out of the affine displacement space.
  // The vector has zero mean, so only its linear projection must be removed:
  // gamma_hat_i = gamma_i - (sum_j gamma_j dx_j)^T A^-1 dx_i.
  Real px = 0.0;
  Real py = 0.0;
  for (const auto i : make_range(4))
  {
    px += _gamma[i] * dx[i](0);
    py += _gamma[i] * dx[i](1);
  }
  const Real projection_x = M00 * px + M01 * py;
  const Real projection_y = M10 * px + M11 * py;

  std::array<Real, 4> projected_gamma;
  for (const auto i : make_range(4))
    projected_gamma[i] = _gamma[i] - projection_x * dx[i](0) - projection_y * dx[i](1);

  // 3) Hourglass amplitude
  Real H = 0.0;
  for (const auto i : make_range(4))
    H += projected_gamma[i] * _v[i];

  // 4) Pointwise scale. Kernel assembly supplies the one-point JxWxT measure, yielding the
  // integrated coefficient penalty * mu * area / h^2 (or the RZ volume measure).
  const Real h2 = std::max((A00 + A11) * 0.5, tiny);
  const Real c = _penalty * _mu / h2;

  // 5) Residual contribution at node _i
  return c * projected_gamma[_i] * H;
}

Real
HourglassCorrectionQuad4::computeQpJacobian()
{
  mooseDoOnce(mooseWarning("This kernel should only be used with explicit time integration."));
  return 0.0;
}
