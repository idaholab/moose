#include "HourglassCorrectionQuad4b.h"
#include "Conversion.h"
#include <algorithm>
#include <cmath>

registerMooseObject("SolidMechanicsApp", HourglassCorrectionQuad4b);

InputParameters
HourglassCorrectionQuad4b::validParams()
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

HourglassCorrectionQuad4b::HourglassCorrectionQuad4b(const InputParameters & parameters)
  : Kernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _mu(getParam<Real>("shear_modulus")),
    _v(_var.dofValues())
{
  // Hourglass mode vectors (unnormalized) for a single displacement component
  _g1 = {1.0, -1.0, 1.0, -1.0};
  _g2 = {1.0, 1.0, -1.0, -1.0};
}

Real
HourglassCorrectionQuad4b::computeQpResidual()
{
  // Ensure single quadrature point integration and a QUAD4 element.
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(_v.size() == 4, "This kernel requires 4 nodal DOF values (QUAD4 elements).");

  // 1) Geometry about centroid and invariant metrics
  const Point center = _current_elem->vertex_average();
  std::vector<Point> coords(4), dx(4);
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
  if (std::fabs(det) > eps)
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

  // 2) Least-squares affine fit: u_affine_i = avg + grad · dx_i
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
HourglassCorrectionQuad4b::computeQpJacobian()
{
  // Recompute geometry terms (held fixed w.r.t. displacement variable)
  const Point center = _current_elem->vertex_average();
  std::vector<Point> dx(4);
  for (unsigned int i = 0; i < 4; ++i)
    dx[i] = _current_elem->node_ref(i) - center;

  Real A00 = 0.0, A01 = 0.0, A11 = 0.0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    const Real x = dx[i](0);
    const Real y = dx[i](1);
    A00 += x * x;
    A01 += x * y;
    A11 += y * y;
  }
  const Real det = A00 * A11 - A01 * A01;
  const Real eps = 1e-12;
  Real M00, M01, M10, M11;
  if (std::fabs(det) > eps)
  {
    const Real inv = 1.0 / det;
    M00 = A11 * inv;
    M01 = -A01 * inv;
    M10 = -A01 * inv;
    M11 = A00 * inv;
  }
  else
  {
    const Real reg = std::max(A00 + A11, eps);
    M00 = 1.0 / std::max(A00, reg);
    M01 = 0.0;
    M10 = 0.0;
    M11 = 1.0 / std::max(A11, reg);
  }

  // Hourglass geometry vectors p_a = sum_i g_a_i dx_i
  Real p1x = 0.0, p1y = 0.0, p2x = 0.0, p2y = 0.0;
  for (unsigned int k = 0; k < 4; ++k)
  {
    p1x += _g1[k] * dx[k](0);
    p1y += _g1[k] * dx[k](1);
    p2x += _g2[k] * dx[k](0);
    p2y += _g2[k] * dx[k](1);
  }

  // c scaling (geometry-only)
  const Real area = _current_elem->volume();
  const Real h2 = std::max((A00 + A11) * 0.5, eps);
  const Real c = _penalty * _mu * (area / h2);

  // Compute s_a(j) = p_a^T M dx_j
  const Real dxjx = dx[_j](0), dxjy = dx[_j](1);
  const Real Mdxjx = M00 * dxjx + M01 * dxjy;
  const Real Mdxjy = M10 * dxjx + M11 * dxjy;
  const Real s1j = p1x * Mdxjx + p1y * Mdxjy;
  const Real s2j = p2x * Mdxjx + p2y * Mdxjy;

  // Consistent tangent: K_ij = c * [ g1_i*(g1_j - s1j) + g2_i*(g2_j - s2j) ]
  return c * (_g1[_i] * (_g1[_j] - s1j) + _g2[_i] * (_g2[_j] - s2j));
}
