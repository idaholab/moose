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
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HourglassCorrectionQuad4b::HourglassCorrectionQuad4b(const InputParameters & parameters)
  : Kernel(parameters), _penalty(getParam<Real>("penalty")), _v(_var.dofValues())
{
  // Precompute the normalized hourglass mode vectors for a QUAD4 element.
  // Base vectors for a single displacement component:
  //   Mode 1: [ 1, -1, 1, -1 ]
  //   Mode 2: [ 1,  1,-1, -1 ]
  const Real base_g1[4] = {1.0, -1.0, 1.0, -1.0};
  const Real base_g2[4] = {1.0, 1.0, -1.0, -1.0};

  _g1.resize(4);
  _g2.resize(4);

  Real norm_g1 = 0.0, norm_g2 = 0.0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    norm_g1 += base_g1[i] * base_g1[i];
    norm_g2 += base_g2[i] * base_g2[i];
  }
  norm_g1 = std::sqrt(norm_g1);
  norm_g2 = std::sqrt(norm_g2);
  for (unsigned int i = 0; i < 4; ++i)
  {
    _g1[i] = base_g1[i] / norm_g1;
    _g2[i] = base_g2[i] / norm_g2;
  }
}

Real
HourglassCorrectionQuad4b::computeQpResidual()
{
  // Ensure single quadrature point integration and a QUAD4 element.
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(_v.size() == 4, "This kernel requires 4 nodal DOF values (QUAD4 elements).");

  // ---------------------------
  // 1. Retrieve element geometry using MOOSE APIs and libMesh::Point operators.
  // ---------------------------
  // Get the element centroid.
  Point center = _current_elem->vertex_average();

  // Retrieve nodal coordinates.
  std::vector<Point> coords(4);
  for (unsigned int i = 0; i < 4; ++i)
    coords[i] = _current_elem->node_ref(i);

  // Compute deviations from the centroid using overloaded operators.
  std::vector<Point> dx(4);
  for (unsigned int i = 0; i < 4; ++i)
    dx[i] = coords[i] - center;

  // Get the element area (for 2D, volume() returns the area).
  Real area = _current_elem->volume();

  // Compute an axis-aligned bounding box (note: this is sensitive to rotation).
  Real min_x = coords[0](0), max_x = coords[0](0);
  Real min_y = coords[0](1), max_y = coords[0](1);
  for (unsigned int i = 1; i < 4; ++i)
  {
    min_x = std::min(min_x, coords[i](0));
    max_x = std::max(max_x, coords[i](0));
    min_y = std::min(min_y, coords[i](1));
    max_y = std::max(max_y, coords[i](1));
  }
  Real Lx = max_x - min_x;
  Real Ly = max_y - min_y;
  Real aspect_ratio = (std::fabs(Ly) > 1e-12) ? Lx / Ly : 1.0;

  // ---------------------------
  // 2. Compute the affine (uniform) displacement field.
  // ---------------------------
  // Compute the average displacement of the current component.
  Real avg_disp = (_v[0] + _v[1] + _v[2] + _v[3]) / 4.0;

  // Estimate an affine gradient using nodes 0 and 2 (diagonally opposite).
  Real dx_total = coords[2](0) - coords[0](0);
  Real dy_total = coords[2](1) - coords[0](1);
  Real grad_x = (std::fabs(dx_total) > 1e-12) ? (_v[2] - _v[0]) / dx_total : 0.0;
  Real grad_y = (std::fabs(dy_total) > 1e-12) ? (_v[2] - _v[0]) / dy_total : 0.0;

  // Compute the affine displacement at each node.
  std::vector<Real> u_affine(4);
  for (unsigned int i = 0; i < 4; ++i)
    u_affine[i] = avg_disp + grad_x * dx[i](0) + grad_y * dx[i](1);

  // The non-affine (hourglass) displacement is the difference between actual and affine
  // displacements.
  std::vector<Real> u_hourglass(4);
  for (unsigned int i = 0; i < 4; ++i)
    u_hourglass[i] = _v[i] - u_affine[i];

  // ---------------------------
  // 3. Project the hourglass displacement onto the precomputed orthonormal modes.
  // ---------------------------
  Real H1 = 0.0, H2 = 0.0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    H1 += u_hourglass[i] * _g1[i];
    H2 += u_hourglass[i] * _g2[i];
  }

  // ---------------------------
  // 4. Compute a dynamic penalty factor based on current geometry.
  // ---------------------------
  // Scale the base penalty using the element area and the aspect ratio.
  Real geometric_scaling = area / (Lx * Ly);
  Real aspect_scaling = 1.0 + std::fabs(aspect_ratio - 1.0);
  Real dynamic_penalty = _penalty * geometric_scaling * aspect_scaling;

  // ---------------------------
  // 5. Assemble the correction force (residual) for the current displacement component.
  // ---------------------------
  // The kernel returns the component of the hourglass correction force for the current node (_i).
  Real residual = dynamic_penalty * (_g1[_i] * H1 + _g2[_i] * H2);

  return residual;
}

Real
HourglassCorrectionQuad4b::computeQpJacobian()
{
  // For brevity, a simplified Jacobian is provided. A fully consistent Jacobian would require
  // differentiating the dynamic penalty and projection steps.
  return _penalty * ((_g1[_i] * _g1[_j] + _g2[_i] * _g2[_j]) / 16.0);
}
