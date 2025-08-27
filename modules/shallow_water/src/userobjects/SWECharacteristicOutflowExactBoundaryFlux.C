//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWECharacteristicOutflowExactBoundaryFlux.h"

registerMooseObject("ShallowWaterApp", SWECharacteristicOutflowExactBoundaryFlux);

InputParameters
SWECharacteristicOutflowExactBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription("Characteristic-based outflow flux for SWE using invariant matching.");
  params.addParam<Real>("gravity", 9.81, "Gravitational acceleration g");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  params.addParam<Real>(
      "target_depth",
      -1.0,
      "Target downstream depth (m) for the incoming characteristic in subcritical outflow."
      " If negative, falls back to pure outflow.");
  params.addParam<Real>(
      "target_un", 0.0, "Target downstream normal velocity (m/s) used with target_depth.");
  params.addParam<Real>(
      "pressure_weight",
      0.0,
      "Weight for the pressure term in the boundary momentum flux (0=off, 1=full)");
  return params;
}

SWECharacteristicOutflowExactBoundaryFlux::SWECharacteristicOutflowExactBoundaryFlux(
    const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _g(getParam<Real>("gravity")),
    _h_eps(getParam<Real>("dry_depth")),
    _target_h(getParam<Real>("target_depth")),
    _target_un(getParam<Real>("target_un")),
    _pressure_weight(getParam<Real>("pressure_weight"))
{
}

SWECharacteristicOutflowExactBoundaryFlux::~SWECharacteristicOutflowExactBoundaryFlux() {}

void
SWECharacteristicOutflowExactBoundaryFlux::calcFlux(unsigned int /*iside*/,
                                                    dof_id_type /*ielem*/,
                                                    const std::vector<Real> & U,
                                                    const RealVectorValue & n,
                                                    std::vector<Real> & flux) const
{
  mooseAssert(U.size() >= 3, "Expected at least 3 conservative variables");
  const Real nx = n(0), ny = n(1);
  const Real tx = -ny, ty = nx;

  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > _h_eps) ? U[1] : 0.0;
  const Real hv = (h > _h_eps) ? U[2] : 0.0;

  const Real un = (h > _h_eps) ? (hu * nx + hv * ny) / h : 0.0;
  const Real ut = (h > _h_eps) ? (hu * tx + hv * ty) / h : 0.0;
  const Real c = std::sqrt(_g * std::max(h, 0.0));

  // if not outflow, no flux
  flux.resize(3);
  if (un <= 0.0)
  {
    flux.assign(3, 0.0);
    return;
  }

  // Helper for physical flux with pressure
  auto Fn = [&](Real hh, Real hhu, Real hhv, Real u_n)
  {
    std::vector<Real> f(3, 0.0);
    f[0] = hh * u_n;
    f[1] = hhu * u_n + _pressure_weight * 0.5 * _g * hh * hh * nx;
    f[2] = hhv * u_n + _pressure_weight * 0.5 * _g * hh * hh * ny;
    return f;
  };

  if (_target_h > 0.0 && un < c)
  {
    // Subcritical: match outgoing char (R+) and impose incoming (R-) from target
    const Real RplusL = un + 2.0 * c;
    const Real c_target = std::sqrt(_g * _target_h);
    const Real Rminus_target = _target_un - 2.0 * c_target;

    const Real unR = 0.5 * (RplusL + Rminus_target);
    const Real cR = 0.25 * (RplusL - Rminus_target);
    const Real hR = std::max(0.0, (cR * cR) / _g);
    const Real huR = hR * (unR * nx + ut * tx);
    const Real hvR = hR * (unR * ny + ut * ty);

    const auto FL = Fn(h, hu, hv, un);
    const auto FR = Fn(hR, huR, hvR, unR);
    const Real smax = std::max(std::fabs(un) + c, std::fabs(unR) + cR);

    for (unsigned int i = 0; i < 3; ++i)
    {
      const Real UR = (i == 0 ? hR : (i == 1 ? huR : hvR));
      const Real UL = (i == 0 ? h : (i == 1 ? hu : hv));
      flux[i] = 0.5 * (FL[i] + FR[i]) - 0.5 * smax * (UR - UL);
    }
    return;
  }

  // Supercritical or no target: pure outflow (extrapolate)
  const auto F = Fn(h, hu, hv, un);
  flux = F;
}

void
SWECharacteristicOutflowExactBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                                        dof_id_type /*ielem*/,
                                                        const std::vector<Real> & U,
                                                        const RealVectorValue & /*n*/,
                                                        DenseMatrix<Real> & J) const
{
  // Approximate diagonal Jacobian for robustness
  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > _h_eps) ? U[1] : 0.0;
  const Real hv = (h > _h_eps) ? U[2] : 0.0;
  const Real un = (h > _h_eps) ? std::sqrt((hu * hu + hv * hv) / (h * h)) : 0.0; // magnitude
  const Real c = std::sqrt(_g * std::max(h, 0.0));
  const Real smax = std::fabs(un) + c;
  J.resize(3, 3);
  J.zero();
  for (unsigned int i = 0; i < 3; ++i)
    J(i, i) = 0.5 * smax;
}
