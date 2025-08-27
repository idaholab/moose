//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEInflowBoundaryFlux.h"

registerMooseObject("ShallowWaterApp", SWEInflowBoundaryFlux);

InputParameters
SWEInflowBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription(
      "Inflow boundary flux for SWE using Rusanov flux with prescribed h and un.");
  params.addParam<Real>("gravity", 9.81, "Gravitational acceleration g");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  params.addRequiredParam<Real>("h_in", "Prescribed inflow depth h");
  params.addRequiredParam<Real>("un_in", "Prescribed inflow normal velocity $u \\cdot n$");
  return params;
}

SWEInflowBoundaryFlux::SWEInflowBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _g(getParam<Real>("gravity")),
    _h_eps(getParam<Real>("dry_depth")),
    _h_in(getParam<Real>("h_in")),
    _un_in(getParam<Real>("un_in"))
{
}

SWEInflowBoundaryFlux::~SWEInflowBoundaryFlux() {}

void
SWEInflowBoundaryFlux::calcFlux(unsigned int /*iside*/,
                                dof_id_type /*ielem*/,
                                const std::vector<Real> & UL,
                                const RealVectorValue & n,
                                std::vector<Real> & flux) const
{
  mooseAssert(UL.size() >= 3, "Expected at least 3 conservative variables");
  const Real nx = n(0), ny = n(1);

  // Build external (ghost) state with normal-only velocity
  const Real hR = std::max(_h_in, 0.0);
  const Real huR = hR * _un_in * nx;
  const Real hvR = hR * _un_in * ny;
  const std::vector<Real> UR = {hR, huR, hvR};

  // Reuse HLL/Rusanov flux logic (no bathymetry here)
  const Real hL = std::max(UL[0], 0.0);
  const Real huL = (hL > _h_eps) ? UL[1] : 0.0;
  const Real hvL = (hL > _h_eps) ? UL[2] : 0.0;

  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real cL = std::sqrt(_g * std::max(hL, 0.0));
  const Real cR = std::sqrt(_g * std::max(hR, 0.0));
  const Real smax = std::max(std::fabs(unL) + cL, std::fabs(unR) + cR);

  auto Fn = [&](Real h, Real hu, Real hv, Real un)
  {
    std::vector<Real> f(3, 0.0);
    f[0] = h * un;
    f[1] = hu * un + 0.5 * _g * h * h * nx;
    f[2] = hv * un + 0.5 * _g * h * h * ny;
    return f;
  };

  const auto FL = Fn(hL, huL, hvL, unL);
  const auto FR = Fn(hR, huR, hvR, unR);

  flux.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    flux[i] = 0.5 * (FL[i] + FR[i]) - 0.5 * smax *
                                          ((i == 0 ? UR[0] : (i == 1 ? UR[1] : UR[2])) -
                                           (i == 0 ? UL[0] : (i == 1 ? UL[1] : UL[2])));
}

void
SWEInflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                    dof_id_type /*ielem*/,
                                    const std::vector<Real> & UL,
                                    const RealVectorValue & n,
                                    DenseMatrix<Real> & jac1) const
{
  mooseAssert(UL.size() >= 3, "Expected at least 3 conservative variables");
  const Real nx = n(0), ny = n(1);
  const Real hL = std::max(UL[0], 0.0);
  const Real huL = (hL > _h_eps) ? UL[1] : 0.0;
  const Real hvL = (hL > _h_eps) ? UL[2] : 0.0;

  // dF/dUL like Rusanov: 0.5*dF(UL) + 0.5*smax*I
  jac1.resize(3, 3);
  jac1.zero();

  if (hL > _h_eps)
  {
    const Real invh = 1.0 / hL;
    const Real un = (huL * nx + hvL * ny) * invh;
    const Real qn = huL * nx + hvL * ny;
    const Real d_un_dh = -qn * invh * invh;
    const Real d_un_dhu = nx * invh;
    const Real d_un_dhv = ny * invh;

    jac1(0, 0) = 0.0;
    jac1(0, 1) = nx;
    jac1(0, 2) = ny;

    jac1(1, 0) = huL * d_un_dh + _g * hL * nx;
    jac1(1, 1) = un + huL * d_un_dhu;
    jac1(1, 2) = huL * d_un_dhv;

    jac1(2, 0) = hvL * d_un_dh + _g * hL * ny;
    jac1(2, 1) = hvL * d_un_dhu;
    jac1(2, 2) = un + hvL * d_un_dhv;
  }

  // smax term
  const Real cL = std::sqrt(_g * std::max(hL, 0.0));
  const Real un = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real hR = std::max(_h_in, 0.0);
  const Real huR = hR * _un_in * nx;
  const Real hvR = hR * _un_in * ny;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real cR = std::sqrt(_g * std::max(hR, 0.0));
  const Real smax = std::max(std::fabs(un) + cL, std::fabs(unR) + cR);
  for (unsigned int i = 0; i < 3; ++i)
    jac1(i, i) += 0.5 * smax;
}
