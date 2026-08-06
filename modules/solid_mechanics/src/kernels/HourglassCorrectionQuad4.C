#include "HourglassCorrectionQuad4.h"
#include "Conversion.h"
#include <algorithm>
#include <cmath>

registerMooseObject("SolidMechanicsApp", HourglassCorrectionQuad4);

InputParameters
HourglassCorrectionQuad4::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Advanced hourglass correction for QUAD4 elements that "
                             "incorporates geometric updates, finite strain effects, and "
                             "enforces orthogonality between hourglass and physical modes.");
  params.addParam<Real>("penalty", 1.0, "Base penalty parameter");
  params.addParam<Real>(
      "shear_modulus",
      1.0,
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
    _g1({1.0, -1.0, 1.0, -1.0}),
    _g2({1.0, 1.0, -1.0, -1.0})
{
}

Real
HourglassCorrectionQuad4::computeQpResidual()
{
  // Ensure single quadrature point integration and a QUAD4 element.
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(_current_elem->type() == libMesh::QUAD4, "This kernel only operates on QUAD4 elements.");
  mooseAssert(_v.size() == 4, "This kernel requires 4 nodal DOF values.");

  // 1) Geometry about centroid and invariant metrics
  const Point center = _current_elem->vertex_average();
  std::array<Point, 4> coords, dx;
  for (unsigned int i = 0; i < 4; ++i)
  {
    coords[i] = _current_elem->node_ref(i);
    dx[i] = coords[i] - center;
  }
  const Real area = _current_elem->volume();

  // Build A = sum_i dx_i dx_i^T
  Real A00 = 0.0, A01 = 0.0, A11 = 0.0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    const Real x = dx[i](0);
    const Real y = dx[i](1);
    A00 += x * x;
    A01 += x * y;
    A11 += y * y;
  }

  // Invert A robustly
  const Real det = A00 * A11 - A01 * A01;
  const Real eps = 1e-12;
  Real M00, M01, M10, M11;
  if (std::abs(det) > eps)
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
    const Real reg = std::max(A00 + A11, eps);
    M00 = 1.0 / std::max(A00, reg);
    M01 = 0.0;
    M10 = 0.0;
    M11 = 1.0 / std::max(A11, reg);
  }

  // 2) Least-squares affine fit: u_affine_i = avg + grad * dx_i
  const Real avg = (_v[0] + _v[1] + _v[2] + _v[3]) / 4.0;
  Real bx = 0.0, by = 0.0; // b = sum_i u_i dx_i
  for (unsigned int i = 0; i < 4; ++i)
  {
    bx += _v[i] * dx[i](0);
    by += _v[i] * dx[i](1);
  }
  const Real gradx = M00 * bx + M01 * by;
  const Real grady = M10 * bx + M11 * by;

  std::vector<Real> u_hg(4);
  for (unsigned int i = 0; i < 4; ++i)
  {
    const Real u_aff = avg + gradx * dx[i](0) + grady * dx[i](1);
    u_hg[i] = _v[i] - u_aff;
  }

  // 3) Hourglass projections
  Real H1 = 0.0, H2 = 0.0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    H1 += _g1[i] * u_hg[i];
    H2 += _g2[i] * u_hg[i];
  }

  // 4) Rotation-invariant scaling c = penalty * area / h^2, with h^2 = trace(A)/2
  const Real h2 = std::max((A00 + A11) * 0.5, eps);
  const Real c = _penalty * _mu * (area / h2);

  // 5) Residual contribution at node _i
  return c * (_g1[_i] * H1 + _g2[_i] * H2);
}

Real
HourglassCorrectionQuad4::computeQpJacobian()
{
  mooseWarning("This kernel should only be used with explicit time integration.");
  return 0.0;
}
