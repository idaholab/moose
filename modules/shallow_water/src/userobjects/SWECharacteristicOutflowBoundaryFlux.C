//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWECharacteristicOutflowBoundaryFlux.h"

registerMooseObject("ShallowWaterApp", SWECharacteristicOutflowBoundaryFlux);

InputParameters
SWECharacteristicOutflowBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription("Characteristic-inspired outflow flux for SWE (advective-only).");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  params.addParam<bool>(
      "outflow_only", true, "If true, emit flux only when un>0 (outflow); zero flux otherwise");
  params.addParam<Real>("ramp_time",
                        0.0,
                        "Optional ramp time from 0 to full advective flux starting at t=0."
                        " If zero, no ramping is applied.");
  params.addParam<unsigned int>(
      "ramp_steps",
      0,
      "Optional number of initial time steps during which the outflow flux is fully suppressed.");
  return params;
}

SWECharacteristicOutflowBoundaryFlux::SWECharacteristicOutflowBoundaryFlux(
    const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _h_eps(getParam<Real>("dry_depth")),
    _outflow_only(getParam<bool>("outflow_only")),
    _ramp_time(getParam<Real>("ramp_time")),
    _ramp_steps(getParam<unsigned int>("ramp_steps"))
{
}

SWECharacteristicOutflowBoundaryFlux::~SWECharacteristicOutflowBoundaryFlux() {}

void
SWECharacteristicOutflowBoundaryFlux::calcFlux(unsigned int /*iside*/,
                                               dof_id_type /*ielem*/,
                                               const std::vector<Real> & U,
                                               const RealVectorValue & n,
                                               std::vector<Real> & flux) const
{
  mooseAssert(U.size() >= 4, "Expected [h,hu,hv,(b),g]");
  const Real nx = n(0), ny = n(1);
  const bool has_b = (U.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  const Real g_here = U[idx_g];
  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > _h_eps) ? U[1] : 0.0;
  const Real hv = (h > _h_eps) ? U[2] : 0.0;

  const Real un = (h > _h_eps) ? (hu * nx + hv * ny) / h : 0.0;

  flux.resize(3);
  // Fully suppress outflow for the first ramp_steps time steps
  if (static_cast<unsigned int>(_fe_problem.timeStep()) <= _ramp_steps)
  {
    flux.assign(3, 0.0);
    return;
  }

  if (_outflow_only && un <= 0.0)
  {
    flux[0] = flux[1] = flux[2] = 0.0;
    return;
  }

  // Advective-only flux projected on n (mass/momentum transport)
  Real scale = 1.0;
  if (_ramp_time > 0.0)
    scale = std::max(0.0, std::min(1.0, _t / _ramp_time));

  const Real adv = (_outflow_only ? std::max(un, 0.0) : un);

  flux[0] = scale * (h * adv);
  flux[1] = scale * ((hu * adv) + 0.5 * g_here * h * h * nx); // <-- add pressure
  flux[2] = scale * ((hv * adv) + 0.5 * g_here * h * h * ny); // <-- add pressure
}

void
SWECharacteristicOutflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                                   dof_id_type /*ielem*/,
                                                   const std::vector<Real> & U,
                                                   const RealVectorValue & n,
                                                   DenseMatrix<Real> & J) const
{
  mooseAssert(U.size() >= 4, "Expected [h,hu,hv,(b),g]");
  const Real nx = n(0), ny = n(1);
  const bool has_b = (U.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  const Real g_here = U[idx_g];
  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > _h_eps) ? U[1] : 0.0;
  const Real hv = (h > _h_eps) ? U[2] : 0.0;

  const Real invh = (h > _h_eps) ? 1.0 / h : 0.0;
  const Real un = (h > _h_eps) ? (hu * nx + hv * ny) * invh : 0.0;

  J.resize(3, 3);
  J.zero();

  if (static_cast<unsigned int>(_fe_problem.timeStep()) <= _ramp_steps ||
      (_outflow_only && un <= 0.0))
    return;

  // Apply same ramp scaling to Jacobian
  Real scale = 1.0;
  if (_ramp_time > 0.0)
    scale = std::max(0.0, std::min(1.0, _t / _ramp_time));

  if (h > _h_eps)
  {
    // Derivatives for advective flux F = [h un, hu un, hv un]
    // dF0/dh = 0 (un depends on h but we ignore for robustness)
    J(0, 1) = scale * nx;
    J(0, 2) = scale * ny;

    const Real d_un_dhu = nx * invh;
    const Real d_un_dhv = ny * invh;
    const Real d_un_dh = -(hu * nx + hv * ny) * invh * invh;

    // F1 = hu*un
    J(1, 0) = scale * (hu * d_un_dh);
    J(1, 1) = scale * (un + hu * d_un_dhu);
    J(1, 2) = scale * (hu * d_un_dhv);

    // F2 = hv*un
    J(2, 0) = scale * (hv * d_un_dh);
    J(2, 1) = scale * (hv * d_un_dhu);
    J(2, 2) = scale * (un + hv * d_un_dhv);

    J(1, 0) += scale * g_here * h * nx;
    J(2, 0) += scale * g_here * h * ny;
  }
}
