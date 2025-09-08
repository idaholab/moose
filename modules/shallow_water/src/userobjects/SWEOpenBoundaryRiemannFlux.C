//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEOpenBoundaryRiemannFlux.h"

registerMooseObject("ShallowWaterApp", SWEOpenBoundaryRiemannFlux);

InputParameters
SWEOpenBoundaryRiemannFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription(
      "Open/outflow boundary flux for SWE using ghost-state Riemann with HLLC/HLL.");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");

  MooseEnum farfield_mode("stage dry", "stage");
  params.addParam<MooseEnum>(
      "farfield_mode", farfield_mode, "Farfield specification mode: stage or dry");
  params.addParam<Real>(
      "eta_infty", 0.0, "Farfield stage (eta=h+b) used when farfield_mode='stage'");
  params.addParam<Real>("u_n_infty", 0.0, "Farfield normal velocity (m/s)");
  params.addParam<Real>("u_t_infty", 0.0, "Farfield tangential velocity (m/s)");
  params.addParam<bool>("use_hllc", true, "If true, use HLLC (else HLL) in internal flux");
  params.addParam<bool>("allow_backflow", true, "Allow backflow at outlet without gating");

  // Reuse the interior numerical flux implementation (with hydrostatic reconstruction)
  params.addRequiredParam<UserObjectName>(
      "numerical_flux",
      "Name of the internal-side numerical flux UserObject (HLLC/HLL) to reuse at the boundary");

  return params;
}

SWEOpenBoundaryRiemannFlux::SWEOpenBoundaryRiemannFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _h_eps(getParam<Real>("dry_depth")),
    _mode(
        [&]()
        {
          const MooseEnum & mm = getParam<MooseEnum>("farfield_mode");
          return (mm == "stage") ? FarfieldMode::Stage : FarfieldMode::Dry;
        }()),
    _eta_infty(getParam<Real>("eta_infty")),
    _un_infty(getParam<Real>("u_n_infty")),
    _ut_infty(getParam<Real>("u_t_infty")),
    _use_hllc(getParam<bool>("use_hllc")),
    _allow_backflow(getParam<bool>("allow_backflow")),
    _riemann_flux(getUserObject<InternalSideFluxBase>("numerical_flux"))
{
}

SWEOpenBoundaryRiemannFlux::~SWEOpenBoundaryRiemannFlux() {}

void
SWEOpenBoundaryRiemannFlux::calcFlux(unsigned int iside,
                                     dof_id_type ielem,
                                     const std::vector<Real> & Uin,
                                     const RealVectorValue & n,
                                     std::vector<Real> & flux) const
{
  mooseAssert(Uin.size() >= 4, "Expected [h,hu,hv,(b),g]");

  // Outward normal and tangential
  const Real nx = n(0), ny = n(1);
  const Real tx = -ny, ty = nx;

  // Interior state (ensure non-negative depth, zero momenta when dry)
  const bool has_b = (Uin.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  const Real g_here = Uin[idx_g];
  const Real b_face = has_b ? Uin[3] : 0.0;
  const Real hL = std::max(Uin[0], 0.0);
  const Real huL = (hL > _h_eps) ? Uin[1] : 0.0;
  const Real hvL = (hL > _h_eps) ? Uin[2] : 0.0;

  // If interior is dry, emit zero flux
  if (hL <= _h_eps)
  {
    flux.assign(3, 0.0);
    return;
  }

  const Real unL = (huL * nx + hvL * ny) / hL;
  const Real utL = (huL * tx + hvL * ty) / hL;
  const Real cL = std::sqrt(g_here * hL);

  // Farfield depth from stage or dry mode
  Real h_inf = 0.0;
  if (_mode == FarfieldMode::Stage)
    h_inf = std::max(_eta_infty - b_face, 0.0);
  else
    h_inf = 0.0; // dry/vacuum

  // Build ghost state (normal/tangential decomposition)
  Real hR = hL;
  Real unR = unL;
  Real utR = utL;

  if (!_allow_backflow && unL < 0.0)
  {
    // Optional gating: suppress inflow by extrapolating (no change)
    hR = hL;
    unR = unL;
    utR = utL;
  }
  else if (unL >= cL)
  {
    // Supercritical outflow: extrapolate interior state
    hR = hL;
    unR = unL;
    utR = utL;
  }
  else if (unL <= -cL)
  {
    // Supercritical inflow: prescribe farfield entirely
    hR = h_inf;
    unR = _un_infty;
    utR = _ut_infty;
  }
  else
  {
    // Subcritical: use 1 incoming char from farfield (R-) and outgoing from interior (R+)
    const Real RplusL = unL + 2.0 * cL;
    const Real c_inf = std::sqrt(g_here * h_inf);
    const Real Rminus_inf = _un_infty - 2.0 * c_inf;
    const Real un_star = 0.5 * (RplusL + Rminus_inf);
    const Real c_star = 0.25 * (RplusL - Rminus_inf);
    const Real h_star = std::max(0.0, (c_star * c_star) / g_here);
    hR = h_star;
    unR = un_star;
    utR = utL; // preserve tangential velocity
  }

  // Convert ghost (normal/tangential) to Cartesian momenta
  const Real huR = hR * (unR * nx + utR * tx);
  const Real hvR = hR * (unR * ny + utR * ty);

  // Left state includes optional bathymetry as fourth entry for hydrostatic reconstruction
  std::vector<Real> UL;
  UL.reserve(4);
  UL.push_back(hL);
  UL.push_back(huL);
  UL.push_back(hvL);
  if (has_b)
    UL.push_back(b_face);

  // Ghost state mirrors bathymetry on boundary to preserve lake-at-rest
  std::vector<Real> UR;
  UR.reserve(4);
  UR.push_back(hR);
  UR.push_back(huR);
  UR.push_back(hvR);
  if (has_b)
    UR.push_back(b_face);

  // Use a synthetic neighbor id keyed by (elem,side) to avoid cache collisions across sides
  const dof_id_type ineig = static_cast<dof_id_type>(ielem * 8 + iside);

  // Reuse interior numerical flux (HLLC/HLL) with hydrostatic reconstruction
  const auto & F = _riemann_flux.getFlux(iside, ielem, ineig, UL, UR, n);
  flux = F;
}

void
SWEOpenBoundaryRiemannFlux::calcJacobian(unsigned int iside,
                                         dof_id_type ielem,
                                         const std::vector<Real> & Uin,
                                         const RealVectorValue & n,
                                         DenseMatrix<Real> & jac1) const
{
  mooseAssert(Uin.size() >= 4, "Expected [h,hu,hv,(b),g]");

  // Outward normal and tangential
  const Real nx = n(0), ny = n(1);
  const Real tx = -ny, ty = nx;

  const bool has_b = (Uin.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  const Real g_here = Uin[idx_g];
  const Real b_face = has_b ? Uin[3] : 0.0;

  const Real hL = std::max(Uin[0], 0.0);
  const Real huL = (hL > _h_eps) ? Uin[1] : 0.0;
  const Real hvL = (hL > _h_eps) ? Uin[2] : 0.0;

  if (hL <= _h_eps)
  {
    jac1.resize(3, 3);
    jac1.zero();
    return;
  }

  const Real unL = (huL * nx + hvL * ny) / hL;
  const Real utL = (huL * tx + hvL * ty) / hL;
  const Real cL = std::sqrt(g_here * hL);

  Real h_inf = 0.0;
  if (_mode == FarfieldMode::Stage)
    h_inf = std::max(_eta_infty - b_face, 0.0);
  else
    h_inf = 0.0;

  Real hR = hL;
  Real unR = unL;
  Real utR = utL;

  if (!_allow_backflow && unL < 0.0)
  {
    // leave as extrapolated
  }
  else if (unL >= cL)
  {
    // extrapolated
  }
  else if (unL <= -cL)
  {
    hR = h_inf;
    unR = _un_infty;
    utR = _ut_infty;
  }
  else
  {
    const Real RplusL = unL + 2.0 * cL;
    const Real c_inf = std::sqrt(g_here * h_inf);
    const Real Rminus_inf = _un_infty - 2.0 * c_inf;
    const Real un_star = 0.5 * (RplusL + Rminus_inf);
    const Real c_star = 0.25 * (RplusL - Rminus_inf);
    const Real h_star = std::max(0.0, (c_star * c_star) / g_here);
    hR = h_star;
    unR = un_star;
    utR = utL;
  }

  const Real huR = hR * (unR * nx + utR * tx);
  const Real hvR = hR * (unR * ny + utR * ty);

  std::vector<Real> UL;
  UL.reserve(4);
  UL.push_back(hL);
  UL.push_back(huL);
  UL.push_back(hvL);
  if (has_b)
    UL.push_back(b_face);

  std::vector<Real> UR;
  UR.reserve(4);
  UR.push_back(hR);
  UR.push_back(huR);
  UR.push_back(hvR);
  if (has_b)
    UR.push_back(b_face);

  const dof_id_type ineig = static_cast<dof_id_type>(ielem * 8 + iside);
  const auto & J = _riemann_flux.getJacobian(Moose::Element, iside, ielem, ineig, UL, UR, n);
  jac1 = J; // copy
}
