//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEWallBoundaryFlux.h"

registerMooseObject("ShallowWaterApp", SWEWallBoundaryFlux);

InputParameters
SWEWallBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription("Solid wall boundary flux for SWE: zero normal velocity.");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  return params;
}

SWEWallBoundaryFlux::SWEWallBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters), _h_eps(getParam<Real>("dry_depth"))
{
}

SWEWallBoundaryFlux::~SWEWallBoundaryFlux() {}

void
SWEWallBoundaryFlux::calcFlux(unsigned int /*iside*/,
                              dof_id_type /*ielem*/,
                              const std::vector<Real> & U,
                              const RealVectorValue & n,
                              std::vector<Real> & flux) const
{
  mooseAssert(U.size() >= 4, "Expected [h,hu,hv,(b),g]");
  // Reflective (solid) wall using a ghost state built by reflecting normal velocity
  const Real nx = n(0), ny = n(1);
  const Real tx = -ny, ty = nx;
  const bool has_b = (U.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  const Real g_here = U[idx_g];
  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > _h_eps) ? U[1] : 0.0;
  const Real hv = (h > _h_eps) ? U[2] : 0.0;

  // Decompose into normal/tangential components
  const Real un = (h > _h_eps) ? (hu * nx + hv * ny) / h : 0.0;
  const Real ut = (h > _h_eps) ? (hu * tx + hv * ty) / h : 0.0;

  // Reflect normal velocity, keep tangential
  const Real unR = -un;
  const Real utR = ut;
  const Real huR = h * (unR * nx + utR * tx);
  const Real hvR = h * (unR * ny + utR * ty);

  // Physical flux projected on n (includes pressure)
  auto Fn = [&](Real hh, Real hhu, Real hhv, Real u_n)
  {
    std::vector<Real> f(3, 0.0);
    f[0] = hh * u_n;
    f[1] = hhu * u_n + 0.5 * g_here * hh * hh * nx;
    f[2] = hhv * u_n + 0.5 * g_here * hh * hh * ny;
    return f;
  };

  const auto FL = Fn(h, hu, hv, un);
  const auto FR = Fn(h, huR, hvR, unR);
  const Real c = std::sqrt(g_here * h);
  const Real smax = std::fabs(un) + c; // symmetric here

  flux.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    flux[i] =
        0.5 * (FL[i] + FR[i]) -
        0.5 * smax * ((i == 0 ? h : (i == 1 ? huR : hvR)) - (i == 0 ? h : (i == 1 ? hu : hv)));
}

void
SWEWallBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                  dof_id_type /*ielem*/,
                                  const std::vector<Real> & U,
                                  const RealVectorValue & n,
                                  DenseMatrix<Real> & J) const
{
  mooseAssert(U.size() >= 3, "Expected at least 3 conservative variables");
  // Approximate Jacobian: diagonal stabilization with smax
  const Real nx = n(0), ny = n(1);
  const Real h = std::max(U[0], 0.0);
  const Real hu = (h > _h_eps) ? U[1] : 0.0;
  const Real hv = (h > _h_eps) ? U[2] : 0.0;
  const bool has_b = (U.size() >= 5);
  const unsigned int idx_g = has_b ? 4 : 3;
  const Real g_here = U[idx_g];

  const Real un = (h > _h_eps) ? (hu * nx + hv * ny) / h : 0.0;
  const Real c = std::sqrt(g_here * h);
  const Real smax = std::fabs(un) + c;
  J.resize(3, 3);
  J.zero();
  for (unsigned int i = 0; i < 3; ++i)
    J(i, i) = 0.5 * smax; // simple, robust approximation
}
