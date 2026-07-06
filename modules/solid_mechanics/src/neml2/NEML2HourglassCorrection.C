//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2HourglassCorrection.h"
#include "FEProblemBase.h"

registerMooseObject("SolidMechanicsApp", NEML2HourglassCorrection);

InputParameters
NEML2HourglassCorrection::validParams()
{
  InputParameters params = NEML2PostKernel::validParams();
  params.addClassDescription(
      "Batched hourglass stabilization for underintegrated QUAD4/HEX8 elements in the NEML2 "
      "nodal-force path: removes the least-squares affine part of the elemental displacement and "
      "penalizes the remaining hourglass modes with a rotation-invariant scale. Equivalent to "
      "HourglassCorrectionQuad4 (and its HEX8 companion) applied per displacement component.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "The displacement variables (2 for QUAD4, 3 for HEX8).");
  params.addParam<Real>("penalty", 1.0, "Base penalty parameter");
  params.addParam<Real>("shear_modulus", 1.0, "Shear modulus scaling the stabilization");
  params.addRequiredParam<std::string>("residual", "The tag for the residual");
  return params;
}

NEML2HourglassCorrection::NEML2HourglassCorrection(const InputParameters & parameters)
  : NEML2PostKernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _mu(getParam<Real>("shear_modulus")),
    _disp_vars(getParam<std::vector<NonlinearVariableName>>("displacements")),
    _ndisp(_disp_vars.size()),
    _rz(_subproblem.getCoordSystem(*_subproblem.mesh().meshSubdomains().begin()) ==
        Moose::COORD_RZ),
    _radial_coord(_rz ? _subproblem.getAxisymmetricRadialCoord() : 0),
    _node_coords(_fe.getNodeCoordinates())
{
  _residual = dynamic_cast<libMesh::PetscVector<Real> *>(
      &_sys.system().add_vector(getParam<std::string>("residual"),
                                /*projections=*/false,
                                libMesh::ParallelType::GHOSTED));
  mooseAssert(_residual, "Failed to cast residual to PetscVector<Real>");

  // a single coordinate system across the mesh is assumed when detecting RZ
  const auto coord = _subproblem.getCoordSystem(*_subproblem.mesh().meshSubdomains().begin());
  for (const auto sid : _subproblem.mesh().meshSubdomains())
    if (_subproblem.getCoordSystem(sid) != coord)
      mooseError("All mesh subdomains must use the same coordinate system.");

  if (_ndisp != 2 && _ndisp != 3)
    paramError(
        "displacements", "2 (QUAD4) or 3 (HEX8) displacement variables required, got ", _ndisp);
  if (_rz && _ndisp != 2)
    paramError("displacements", "RZ hourglass correction requires 2 displacement variables.");

  for (const auto & v : _disp_vars)
    _disp_nodal.push_back(&_fe.getNodalValue(v));

  // hourglass base vectors in libMesh node ordering
  if (_ndisp == 2)
    // QUAD4: matches HourglassCorrectionQuad4
    _gamma = neml2::Tensor(
        torch::tensor({{1., -1., 1., -1.}, {1., 1., -1., -1.}}, torch::dtype(torch::kFloat64)), 0);
  else
    // HEX8: Flanagan-Belytschko patterns xi*eta, eta*zeta, xi*zeta, xi*eta*zeta
    _gamma = neml2::Tensor(torch::tensor({{1., -1., 1., -1., 1., -1., 1., -1.},
                                          {1., 1., -1., -1., -1., -1., 1., 1.},
                                          {1., -1., -1., 1., -1., 1., 1., -1.},
                                          {-1., 1., -1., 1., 1., -1., 1., -1.}},
                                         torch::dtype(torch::kFloat64)),
                           0);
}

void
NEML2HourglassCorrection::forward()
{
  torch::InferenceMode guard;

  const auto nnode_expect = _ndisp == 2 ? 4 : 8;
  const auto X = at::Tensor(_node_coords); // (nelem, nnode, 3)
  if (X.size(1) != nnode_expect)
    mooseError("NEML2HourglassCorrection: expected ",
               nnode_expect,
               " nodes per element, got ",
               X.size(1),
               ". Only QUAD4 (2D) and HEX8 (3D) meshes are supported.");

  const auto gamma = at::Tensor(_gamma).to(X.options()); // (nmode, nnode)

  // current (displaced) coordinates of the active dimensions
  std::vector<at::Tensor> u;
  for (const auto k : make_range(_ndisp))
    u.push_back(at::Tensor(*_disp_nodal[k])); // (nelem, nnode)
  auto x =
      X.index(
           {at::indexing::Slice(), at::indexing::Slice(), at::indexing::Slice(0, int64_t(_ndisp))})
          .clone();
  for (const auto k : make_range(_ndisp))
    x.index_put_({at::indexing::Slice(), at::indexing::Slice(), int64_t(k)},
                 x.index({at::indexing::Slice(), at::indexing::Slice(), int64_t(k)}) + u[k]);

  // geometry about the vertex average
  const auto center = x.mean(1, /*keepdim=*/true);   // (nelem, 1, dim)
  const auto d = x - center;                         // (nelem, nnode, dim)
  const auto A = at::einsum("eni,enj->eij", {d, d}); // (nelem, dim, dim)
  const auto det = at::linalg_det(A);                // (nelem)
  const auto eps = 1e-12;

  // robust inverse: regularized diagonal fallback where |det| <= eps (mirrors
  // HourglassCorrectionQuad4)
  const auto singular = det.abs() <= eps; // (nelem)
  const auto Adiag = at::diag_embed(at::diagonal(A, 0, -2, -1).clamp_min(eps));
  const auto A_safe = at::where(singular.unsqueeze(-1).unsqueeze(-1), Adiag, A);
  const auto M = at::linalg_inv(A_safe); // (nelem, dim, dim)

  // rotation-invariant scale c = penalty * mu * (V / h^2), h^2 = tr(A)/dim
  const auto h2 = (at::diagonal(A, 0, -2, -1).sum(-1) / double(_ndisp)).clamp_min(eps); // (nelem)
  at::Tensor V; // element area (2D) or effective volume (3D)
  if (_ndisp == 2)
  {
    // shoelace area of the (CCW ordered) displaced quad
    const auto xr = x.index({at::indexing::Slice(), at::indexing::Slice(), 0});
    const auto yr = x.index({at::indexing::Slice(), at::indexing::Slice(), 1});
    const auto xs = at::roll(xr, -1, 1);
    const auto ys = at::roll(yr, -1, 1);
    V = 0.5 * (xr * ys - xs * yr).sum(1).abs();
  }
  else
  {
    // effective volume 8 * det(B), B_j = (1/8) sum_i s_j,i x_i: the one-point
    // quadrature JxW of the trilinear map (exact for parallelepipeds)
    const auto signs = torch::tensor({{-1., 1., 1., -1., -1., 1., 1., -1.},
                                      {-1., -1., 1., 1., -1., -1., 1., 1.},
                                      {-1., -1., -1., -1., 1., 1., 1., 1.}},
                                     torch::dtype(torch::kFloat64))
                           .to(X.options()) /
                       8.0;
    const auto B = at::einsum("mn,enj->emj", {signs, x});
    V = 8.0 * at::linalg_det(B).abs();
  }
  auto c = _penalty * _mu * (V / h2); // (nelem)

  // integration weight of the single quadrature point on the displaced mesh
  auto W = V.clone();
  if (_rz)
  {
    const auto rbar =
        x.index({at::indexing::Slice(), at::indexing::Slice(), int64_t(_radial_coord)}).mean(1);
    W = W * (2.0 * libMesh::pi) * rbar;
  }
  const auto cW = (c * W).unsqueeze(-1); // (nelem, 1)

  // per component: least-squares affine fit, hourglass projection, penalty force
  for (const auto k : make_range(_ndisp))
  {
    const auto avg = u[k].mean(1, /*keepdim=*/true);           // (nelem, 1)
    const auto b = at::einsum("en,eni->ei", {u[k], d});        // (nelem, dim)
    const auto g = at::einsum("eij,ej->ei", {M, b});           // (nelem, dim)
    const auto u_aff = avg + at::einsum("ei,eni->en", {g, d}); // (nelem, nnode)
    const auto u_hg = u[k] - u_aff;                            // (nelem, nnode)
    const auto H = at::einsum("mn,en->em", {gamma, u_hg});     // (nelem, nmode)
    const auto re = cW * at::einsum("em,mn->en", {H, gamma});  // (nelem, nnode)

    auto re_c = re.contiguous().cpu();
    const auto & dofmap = _fe.getGlobalDofMap(_disp_vars[k]);
    _residual->add_vector(re_c.data_ptr<Real>(), dofmap);
  }
  _residual->close();
}

#endif
