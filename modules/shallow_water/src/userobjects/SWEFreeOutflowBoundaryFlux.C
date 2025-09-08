//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEFreeOutflowBoundaryFlux.h"

registerMooseObject("ShallowWaterApp", SWEFreeOutflowBoundaryFlux);

InputParameters
SWEFreeOutflowBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription("Free outflow boundary flux for SWE: $F(U) \\cdot n$.");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  return params;
}

SWEFreeOutflowBoundaryFlux::SWEFreeOutflowBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters), _h_eps(getParam<Real>("dry_depth"))
{
}

SWEFreeOutflowBoundaryFlux::~SWEFreeOutflowBoundaryFlux() {}

void
SWEFreeOutflowBoundaryFlux::calcFlux(unsigned int /*iside*/,
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
  flux[0] = h * un;
  flux[1] = hu * un + 0.5 * g_here * h * h * nx;
  flux[2] = hv * un + 0.5 * g_here * h * h * ny;
}

void
SWEFreeOutflowBoundaryFlux::fill_dF(
    const std::vector<Real> & U, Real nx, Real ny, Real g, Real h_eps, DenseMatrix<Real> & J) const
{
  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > h_eps) ? U[1] : 0.0;
  const Real hv = (h > h_eps) ? U[2] : 0.0;
  const Real invh = (h > h_eps) ? 1.0 / h : 0.0;
  const Real un = (h > h_eps) ? (hu * nx + hv * ny) * invh : 0.0;

  J.resize(3, 3);
  // dF0
  J(0, 0) = 0.0;
  J(0, 1) = nx;
  J(0, 2) = ny;

  if (h > h_eps)
  {
    const Real qn = hu * nx + hv * ny;
    const Real d_un_dh = -qn * invh * invh;
    const Real d_un_dhu = nx * invh;
    const Real d_un_dhv = ny * invh;
    // F1
    J(1, 0) = hu * d_un_dh + g * h * nx;
    J(1, 1) = un + hu * d_un_dhu;
    J(1, 2) = hu * d_un_dhv;
    // F2
    J(2, 0) = hv * d_un_dh + g * h * ny;
    J(2, 1) = hv * d_un_dhu;
    J(2, 2) = un + hv * d_un_dhv;
  }
  else
  {
    J(1, 0) = 0.0;
    J(1, 1) = 0.0;
    J(1, 2) = 0.0;
    J(2, 0) = 0.0;
    J(2, 1) = 0.0;
    J(2, 2) = 0.0;
  }
}

void
SWEFreeOutflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                         dof_id_type /*ielem*/,
                                         const std::vector<Real> & U,
                                         const RealVectorValue & n,
                                         DenseMatrix<Real> & jac1) const
{
  mooseAssert(U.size() >= 4, "Expected [h,hu,hv,(b),g]");
  const bool has_b = (U.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  fill_dF(U, n(0), n(1), U[idx_g], _h_eps, jac1);
}
