//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeReconstructionMultiDSWE.h"

#include "libmesh/elem.h"

registerMooseObject("ShallowWaterApp", SlopeReconstructionMultiDSWE);

InputParameters
SlopeReconstructionMultiDSWE::validParams()
{
  InputParameters params = SlopeReconstructionMultiD::validParams();
  params.addClassDescription(
      "Multi-D least-squares reconstruction for SWE variables (h, hu, hv) with optional "
      "positivity guard and side-geometry caching.");

  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");

  params.addParam<unsigned int>("min_neighbors", 2, "Minimum neighbor faces for LSQ gradients");

  MooseEnum weight_model("none inverse_distance2", "inverse_distance2");
  params.addParam<MooseEnum>("weight_model", weight_model, "Neighbor weight model for LSQ");

  params.addParam<Real>("tikhonov_eps", 1e-12, "Diagonal regularization for normal equations");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold to consider a cell wet");
  params.addParam<bool>("positivity_guard",
                        true,
                        "Ensure reconstructed h at face centroids is non-negative");
  params.addParam<Real>("positivity_eps", 1e-12, "Small epsilon for positivity guard");

  return params;
}

SlopeReconstructionMultiDSWE::SlopeReconstructionMultiDSWE(const InputParameters & parameters)
  : SlopeReconstructionMultiD(parameters),
    _h_var(getVar("h", 0)),
    _hu_var(getVar("hu", 0)),
    _hv_var(getVar("hv", 0)),
    _min_neighbors(getParam<unsigned int>("min_neighbors")),
    _weight_model(getParam<MooseEnum>("weight_model")),
    _tikhonov_eps(getParam<Real>("tikhonov_eps")),
    _dry_depth(getParam<Real>("dry_depth")),
    _positivity_guard(getParam<bool>("positivity_guard")),
    _positivity_eps(getParam<Real>("positivity_eps"))
{
}

Real
SlopeReconstructionMultiDSWE::weight(const Point & dx) const
{
  if (_weight_model == "none")
    return 1.0;
  const Real r2 = dx(0) * dx(0) + dx(1) * dx(1);
  return r2 > 0.0 ? 1.0 / r2 : 1.0;
}

void
SlopeReconstructionMultiDSWE::reconstructElementSlope()
{
  const Elem * elem = _current_elem;
  const dof_id_type id = elem->id();

  // Cell center and averages
  const Point xc = elem->vertex_average();

  const bool implicit = _is_implicit; // from TransientInterface via ElementLoopUserObject
  const Real hi = implicit ? _h_var->getElementalValue(elem) : _h_var->getElementalValueOld(elem);
  const Real hui = implicit ? _hu_var->getElementalValue(elem) : _hu_var->getElementalValueOld(elem);
  const Real hvi = implicit ? _hv_var->getElementalValue(elem) : _hv_var->getElementalValueOld(elem);

  std::vector<Real> u0 = {hi, hui, hvi};

  // Prepare normal equations A g = b for each variable (2D => 2x2)
  Real A11 = 0., A12 = 0., A22 = 0.;
  Real b1_h = 0., b2_h = 0.;
  Real b1_hu = 0., b2_hu = 0.;
  Real b1_hv = 0., b2_hv = 0.;

  unsigned int n_neigh = 0;

  // Cache side geometry if not cached yet
  const bool need_side_cache = !_side_geoinfo_cached;

  for (unsigned int s = 0; s < elem->n_sides(); ++s)
  {
    const Elem * neigh = elem->neighbor_ptr(s);
    if (neigh && !this->hasBlocks(neigh->subdomain_id()))
      neigh = nullptr; // treat as boundary if neighbor not in our block set

    // Side centroid/area caching
    if (need_side_cache)
    {
      std::unique_ptr<const Elem> side = elem->build_side_ptr(s);
      const Point sc = side->vertex_average();
      const Real sa = side->volume();
      if (neigh)
      {
        _side_centroid[std::make_pair(id, neigh->id())] = sc;
        _side_area[std::make_pair(id, neigh->id())] = sa;
      }
      else
      {
        _bnd_side_centroid[std::make_pair(id, s)] = sc;
        _bnd_side_area[std::make_pair(id, s)] = sa;
      }
    }

    if (!neigh)
      continue;

    // neighbor center and values
    const Point xn = neigh->vertex_average();
    const Point dx = xn - xc;
    const Real w = weight(dx);

    const Real hn = implicit ? _h_var->getElementalValue(neigh) : _h_var->getElementalValueOld(neigh);
    const Real hun = implicit ? _hu_var->getElementalValue(neigh)
                              : _hu_var->getElementalValueOld(neigh);
    const Real hvn = implicit ? _hv_var->getElementalValue(neigh)
                              : _hv_var->getElementalValueOld(neigh);

    const Real du_h = hn - hi;
    const Real du_hu = hun - hui;
    const Real du_hv = hvn - hvi;

    // accumulate A and b
    const Real dx0 = dx(0), dx1 = dx(1);
    A11 += w * dx0 * dx0;
    A12 += w * dx0 * dx1;
    A22 += w * dx1 * dx1;

    b1_h += w * du_h * dx0;
    b2_h += w * du_h * dx1;

    b1_hu += w * du_hu * dx0;
    b2_hu += w * du_hu * dx1;

    b1_hv += w * du_hv * dx0;
    b2_hv += w * du_hv * dx1;

    ++n_neigh;
  }

  // Prepare output gradients
  RealGradient gh(0., 0., 0.);
  RealGradient ghu(0., 0., 0.);
  RealGradient ghv(0., 0., 0.);

  // Solve if sufficient neighbors and matrix is ok
  if (n_neigh >= _min_neighbors)
  {
    // Regularize and invert 2x2
    const Real a11 = A11 + _tikhonov_eps;
    const Real a12 = A12;
    const Real a22 = A22 + _tikhonov_eps;
    const Real det = a11 * a22 - a12 * a12;

    if (std::abs(det) > 1e-20)
    {
      const Real inv11 = a22 / det;
      const Real inv12 = -a12 / det;
      const Real inv22 = a11 / det;

      gh(0) = inv11 * b1_h + inv12 * b2_h;
      gh(1) = inv12 * b1_h + inv22 * b2_h;

      ghu(0) = inv11 * b1_hu + inv12 * b2_hu;
      ghu(1) = inv12 * b1_hu + inv22 * b2_hu;

      ghv(0) = inv11 * b1_hv + inv12 * b2_hv;
      ghv(1) = inv12 * b1_hv + inv22 * b2_hv;
    }
  }

  // Zero momentum gradients for near-dry cells
  if (hi < _dry_depth)
  {
    ghu = RealGradient(0., 0., 0.);
    ghv = RealGradient(0., 0., 0.);
  }

  // Positivity guard for h at face centroids
  if (_positivity_guard)
  {
    Real phi = 1.0;
    for (unsigned int s = 0; s < elem->n_sides(); ++s)
    {
      const Elem * neigh = elem->neighbor_ptr(s);
      const Point xs = neigh ? _side_centroid.at(std::make_pair(id, neigh->id()))
                             : _bnd_side_centroid.at(std::make_pair(id, s));
      const Real du = gh * (xs - xc);
      const Real hface = hi + du;
      if (hface < _dry_depth + _positivity_eps && du < 0.0)
      {
        const Real phis = (hi - (_dry_depth + _positivity_eps)) / (-du);
        phi = std::min(phi, std::max(0.0, phis));
      }
    }
    phi = std::max(0.0, std::min(1.0, phi));
    gh *= phi;
  }

  // Store results
  _rslope[id] = {gh, ghu, ghv};
  _avars[id] = u0;
}
