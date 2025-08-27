//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWENumericalFluxHLL.h"

registerMooseObject("ShallowWaterApp", SWENumericalFluxHLL);

InputParameters
SWENumericalFluxHLL::validParams()
{
  InputParameters params = SWENumericalFluxBase::validParams();
  params.addClassDescription(
      "HLL/Rusanov-style numerical flux for 2D shallow-water equations (stub).");
  return params;
}

SWENumericalFluxHLL::SWENumericalFluxHLL(const InputParameters & parameters)
  : SWENumericalFluxBase(parameters)
{
}

SWENumericalFluxHLL::~SWENumericalFluxHLL() {}

void
SWENumericalFluxHLL::calcFlux(unsigned int /*iside*/,
                              dof_id_type /*ielem*/,
                              dof_id_type /*ineig*/,
                              const std::vector<Real> & uvec1,
                              const std::vector<Real> & uvec2,
                              const RealVectorValue & n,
                              std::vector<Real> & flux) const
{
  mooseAssert(uvec1.size() >= 3, "Expected at least 3 conservative variables on left");
  mooseAssert(uvec2.size() >= 3, "Expected at least 3 conservative variables on right");

  const Real nx = n(0);
  const Real ny = n(1);

  // Optional hydrostatic reconstruction if bathymetry b is provided
  const bool has_b = (uvec1.size() >= 4 && uvec2.size() >= 4);
  Real hL = std::max(uvec1[0], 0.0);
  Real huL = (hL > _h_eps) ? uvec1[1] : 0.0;
  Real hvL = (hL > _h_eps) ? uvec1[2] : 0.0;
  Real hR = std::max(uvec2[0], 0.0);
  Real huR = (hR > _h_eps) ? uvec2[1] : 0.0;
  Real hvR = (hR > _h_eps) ? uvec2[2] : 0.0;

  if (has_b)
  {
    const Real bL = uvec1[3];
    const Real bR = uvec2[3];
    const Real etaL = hL + bL;
    const Real etaR = hR + bR;
    const Real bstar = std::max(bL, bR);
    const Real hLstar = std::max(0.0, etaL - bstar);
    const Real hRstar = std::max(0.0, etaR - bstar);
    const Real uL = (hL > _h_eps) ? huL / hL : 0.0;
    const Real vL = (hL > _h_eps) ? hvL / hL : 0.0;
    const Real uR = (hR > _h_eps) ? huR / hR : 0.0;
    const Real vR = (hR > _h_eps) ? hvR / hR : 0.0;
    hL = hLstar;
    hR = hRstar;
    huL = uL * hLstar;
    hvL = vL * hLstar;
    huR = uR * hRstar;
    hvR = vR * hRstar;
  }

  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real cL = std::sqrt(_g * std::max(hL, 0.0));
  const Real cR = std::sqrt(_g * std::max(hR, 0.0));
  const Real smax = std::max(std::fabs(unL) + cL, std::fabs(unR) + cR);

  // Physical flux projected on n (includes pressure term)
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
    flux[i] =
        0.5 * (FL[i] + FR[i]) -
        0.5 * smax * ((i == 0 ? hR : (i == 1 ? huR : hvR)) - (i == 0 ? hL : (i == 1 ? huL : hvL)));

  // No extra correction here; hydrostatic reconstruction above adjusts h
  // to maintain lake-at-rest when paired with this physical flux.
}

void
SWENumericalFluxHLL::calcJacobian(unsigned int /*iside*/,
                                  dof_id_type /*ielem*/,
                                  dof_id_type /*ineig*/,
                                  const std::vector<Real> & uvec1,
                                  const std::vector<Real> & uvec2,
                                  const RealVectorValue & n,
                                  DenseMatrix<Real> & jac1,
                                  DenseMatrix<Real> & jac2) const
{
  mooseAssert(uvec1.size() >= 3, "Expected at least 3 conservative variables on left");
  mooseAssert(uvec2.size() >= 3, "Expected at least 3 conservative variables on right");
  jac1.resize(3, 3);
  jac2.resize(3, 3);
  // Approximate analytic Jacobians: linearization of Rusanov flux
  // Treat smax as constant w.r.t. U for robustness; sufficient for Newton.
  // Derivatives are 0.5*(dF_L/dU_L) + 0.5*(dF_R/dU_L) - 0.5*smax*(dU_R/dU_L - dU_L/dU_L)
  // which reduces to 0.5*dF_L/dU_L + 0.5*smax*I for left, and
  // 0.5*dF_R/dU_R + 0.5*smax*I for right, with sign conventions handled by kernel.

  jac1.zero();
  jac2.zero();

  // Build dF/dU for each side assuming U=[h,hu,hv]
  auto fill_dF =
      [&](const std::vector<Real> & U, const Real nx, const Real ny, DenseMatrix<Real> & J)
  {
    const Real h = std::max(U[0], 0.0);
    const Real hu = (h > _h_eps) ? U[1] : 0.0;
    const Real hv = (h > _h_eps) ? U[2] : 0.0;
    const Real invh = (h > _h_eps) ? 1.0 / h : 0.0;
    const Real un = (h > _h_eps) ? (hu * nx + hv * ny) * invh : 0.0;

    // F_n components
    // F0 = h*un = (hu*nx + hv*ny)
    // F1 = hu*un + 0.5*g*h^2*nx = hu*(hu*nx+hv*ny)/h + 0.5*g*h^2*nx
    // F2 = hv*un + 0.5*g*h^2*ny = hv*(hu*nx+hv*ny)/h + 0.5*g*h^2*ny

    // dF0/dh = 0
    // dF0/dhu = nx; dF0/dhv = ny
    J(0, 0) = 0.0;
    J(0, 1) = nx;
    J(0, 2) = ny;

    // For momentum components, guard against dry state
    if (h > _h_eps)
    {
      // common terms
      const Real qn = (hu * nx + hv * ny);
      const Real d_un_dh = -qn * invh * invh;
      const Real d_un_dhu = nx * invh;
      const Real d_un_dhv = ny * invh;

      // F1 = hu*un + 0.5*g*h^2*nx
      // dF1/dh = hu*d_un/dh + g*h*nx
      // dF1/dhu = un + hu*d_un/dhu
      // dF1/dhv = hu*d_un/dhv
      J(1, 0) = hu * d_un_dh + _g * h * nx;
      J(1, 1) = un + hu * d_un_dhu;
      J(1, 2) = hu * d_un_dhv;

      // F2 = hv*un + 0.5*g*h^2*ny
      // dF2/dh = hv*d_un/dh + g*h*ny
      // dF2/dhu = hv*d_un/dhu
      // dF2/dhv = un + hv*d_un/dhv
      J(2, 0) = hv * d_un_dh + _g * h * ny;
      J(2, 1) = hv * d_un_dhu;
      J(2, 2) = un + hv * d_un_dhv;
    }
    else
    {
      // Dry: flux reduces to pressure terms, but we zeroed hu,hv
      J(1, 0) = 0.0;
      J(1, 1) = 0.0;
      J(1, 2) = 0.0;
      J(2, 0) = 0.0;
      J(2, 1) = 0.0;
      J(2, 2) = 0.0;
    }
  };

  // Recompute smax consistent with calcFlux to add 0.5*smax*I contribution
  const Real nx = n(0);
  const Real ny = n(1);
  // Reuse hydrostatic reconstructed states consistent with calcFlux by rebuilding
  const bool has_b = (uvec1.size() >= 4 && uvec2.size() >= 4);
  Real hL = std::max(uvec1[0], 0.0);
  Real huL = (hL > _h_eps) ? uvec1[1] : 0.0;
  Real hvL = (hL > _h_eps) ? uvec1[2] : 0.0;
  Real hR = std::max(uvec2[0], 0.0);
  Real huR = (hR > _h_eps) ? uvec2[1] : 0.0;
  Real hvR = (hR > _h_eps) ? uvec2[2] : 0.0;
  if (has_b)
  {
    const Real bL = uvec1[3];
    const Real bR = uvec2[3];
    const Real etaL = hL + bL;
    const Real etaR = hR + bR;
    const Real bstar = std::max(bL, bR);
    const Real hLstar = std::max(0.0, etaL - bstar);
    const Real hRstar = std::max(0.0, etaR - bstar);
    const Real uL = (hL > _h_eps) ? huL / hL : 0.0;
    const Real vL = (hL > _h_eps) ? hvL / hL : 0.0;
    const Real uR = (hR > _h_eps) ? huR / hR : 0.0;
    const Real vR = (hR > _h_eps) ? hvR / hR : 0.0;
    hL = hLstar;
    hR = hRstar;
    huL = uL * hLstar;
    hvL = vL * hLstar;
    huR = uR * hRstar;
    hvR = vR * hRstar;
  }
  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real cL = std::sqrt(_g * std::max(hL, 0.0));
  const Real cR = std::sqrt(_g * std::max(hR, 0.0));
  const Real smax = std::max(std::fabs(unL) + cL, std::fabs(unR) + cR);

  fill_dF(uvec1, nx, ny, jac1);
  fill_dF(uvec2, nx, ny, jac2);

  // Add 0.5*smax*I factor to approximate Rusanov derivative
  for (unsigned int i = 0; i < 3; ++i)
  {
    jac1(i, i) += 0.5 * smax;
    jac2(i, i) += 0.5 * smax;
  }
}
