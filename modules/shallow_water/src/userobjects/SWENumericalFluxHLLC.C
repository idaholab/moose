//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWENumericalFluxHLLC.h"

registerMooseObject("ShallowWaterApp", SWENumericalFluxHLLC);

InputParameters
SWENumericalFluxHLLC::validParams()
{
  InputParameters params = SWENumericalFluxBase::validParams();
  params.addClassDescription(
      "HLLC numerical flux for 2D shallow-water equations with hydrostatic reconstruction.");
  params.addParam<bool>(
      "use_pvrs", true, "Use PVRS star-depth/q-scaled wave speeds (else Einfeldt)");
  params.addParam<Real>(
      "degeneracy_eps", 1e-8, "Small epsilon for degeneracy/positivity tests and S* clamping");
  params.addParam<Real>(
      "blend_alpha", 0.0, "Optional compression-based blend to HLLE (0 disables; 0.2-0.3 typical)");
  params.addParam<bool>("log_debug", false, "If true, log fallback/blend counters each step");
  return params;
}

SWENumericalFluxHLLC::SWENumericalFluxHLLC(const InputParameters & parameters)
  : SWENumericalFluxBase(parameters),
    _use_pvrs(getParam<bool>("use_pvrs")),
    _degeneracy_eps(getParam<Real>("degeneracy_eps")),
    _blend_alpha(getParam<Real>("blend_alpha")),
    _log_debug(getParam<bool>("log_debug")),
    _fallback_count(0),
    _blend_count(0)
{
}

SWENumericalFluxHLLC::~SWENumericalFluxHLLC() {}

void
SWENumericalFluxHLLC::calcFlux(unsigned int /*iside*/,
                               dof_id_type /*ielem*/,
                               dof_id_type /*ineig*/,
                               const std::vector<Real> & uvec1,
                               const std::vector<Real> & uvec2,
                               const RealVectorValue & n,
                               std::vector<Real> & flux) const
{
  mooseAssert(uvec1.size() >= 3 && uvec2.size() >= 3, "Need [h,hu,hv](,b)");

  const Real nx = n(0), ny = n(1);
  const Real tx = -ny, ty = nx;

  // --- hydrostatic reconstruction (Audusse) if b is present
  const bool has_b = (uvec1.size() >= 4 && uvec2.size() >= 4);
  Real hL = std::max(uvec1[0], 0.0), huL = (hL > _h_eps) ? uvec1[1] : 0.0,
       hvL = (hL > _h_eps) ? uvec1[2] : 0.0;
  Real hR = std::max(uvec2[0], 0.0), huR = (hR > _h_eps) ? uvec2[1] : 0.0,
       hvR = (hR > _h_eps) ? uvec2[2] : 0.0;

  if (has_b)
  {
    const Real bL = uvec1[3], bR = uvec2[3];
    const Real etaL = hL + bL, etaR = hR + bR, bstar = std::max(bL, bR);
    const Real hLstar = std::max(0.0, etaL - bstar);
    const Real hRstar = std::max(0.0, etaR - bstar);
    const Real uLx = (hL > _h_eps) ? huL / hL : 0.0, uLy = (hL > _h_eps) ? hvL / hL : 0.0;
    const Real uRx = (hR > _h_eps) ? huR / hR : 0.0, uRy = (hR > _h_eps) ? hvR / hR : 0.0;
    hL = hLstar;
    hR = hRstar;
    huL = uLx * hL;
    hvL = uLy * hL;
    huR = uRx * hR;
    hvR = uRy * hR;
  }

  // decompose velocity
  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real utL = (hL > _h_eps) ? (huL * tx + hvL * ty) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real utR = (hR > _h_eps) ? (huR * tx + hvR * ty) / hR : 0.0;
  const Real cL = std::sqrt(_g * hL);
  const Real cR = std::sqrt(_g * hR);

  // Wave speeds
  Real SL, SR;
  if (_use_pvrs)
  {
    const Real aL = cL, aR = cR;
    const Real a_sum = std::max(1e-12, aL + aR);
    // PVRS star-depth estimate (heuristic for SWE)
    const Real h_pvrs = std::max(0.0, 0.5 * (hL + hR) - 0.25 * (unR - unL) * (hL + hR) / a_sum);
    const Real qL =
        (h_pvrs <= hL || hL <= _h_eps) ? 1.0 : std::sqrt(1.0 + std::max(0.0, h_pvrs / hL - 1.0));
    const Real qR =
        (h_pvrs <= hR || hR <= _h_eps) ? 1.0 : std::sqrt(1.0 + std::max(0.0, h_pvrs / hR - 1.0));
    SL = unL - aL * qL;
    SR = unR + aR * qR;
  }
  else
  {
    // Einfeldt/Davis bounds
    SL = std::min(unL - cL, unR - cR);
    SR = std::max(unL + cL, unR + cR);
  }

  // contact speed S*  (includes pressure jump)
  // Toro (adapted to SWE normal dir):
  // S* = [ (SR - unR) hR unR - (SL - unL) hL unL + 0.5*g*(hL^2 - hR^2) ]
  //      / [ (SR - unR) hR       - (SL - unL) hL ]
  const Real AL = (SL - unL), AR = (SR - unR);
  const Real denom = AR * hR - AL * hL;
  Real Sstar;
  if (std::abs(denom) > 1e-12)
    Sstar = (AR * hR * unR - AL * hL * unL + 0.5 * _g * (hL * hL - hR * hR)) / denom;
  else
    Sstar = 0.5 * (unL + unR);
  // clamp S* within fan to avoid degeneracy
  Sstar = std::max(SL + _degeneracy_eps, std::min(SR - _degeneracy_eps, Sstar));

  // star depths (positivity clamp)
  const Real denomL = (SL - Sstar);
  const Real denomR = (SR - Sstar);
  Real hLstar =
      hL * (SL - unL) /
      (std::abs(denomL) > _degeneracy_eps ? denomL : std::copysign(_degeneracy_eps, denomL));
  Real hRstar =
      hR * (SR - unR) /
      (std::abs(denomR) > _degeneracy_eps ? denomR : std::copysign(_degeneracy_eps, denomR));
  hLstar = std::max(0.0, hLstar);
  hRstar = std::max(0.0, hRstar);

  // physical flux in n-direction
  auto Fn = [&](Real h, Real hu, Real hv, Real un)
  {
    std::vector<Real> f(3, 0.0);
    f[0] = h * un;
    f[1] = hu * un + 0.5 * _g * h * h * nx;
    f[2] = hv * un + 0.5 * _g * h * h * ny;
    return f;
  };

  const auto UL = std::vector<Real>{hL, huL, hvL};
  const auto UR = std::vector<Real>{hR, huR, hvR};
  const auto FL = Fn(hL, huL, hvL, unL);
  const auto FR = Fn(hR, huR, hvR, unR);

  // star momenta: S* for normal, ut unchanged
  const Real huLstar = hLstar * (Sstar * nx + utL * tx);
  const Real hvLstar = hLstar * (Sstar * ny + utL * ty);
  const Real huRstar = hRstar * (Sstar * nx + utR * tx);
  const Real hvRstar = hRstar * (Sstar * ny + utR * ty);

  const auto ULs = std::vector<Real>{hLstar, huLstar, hvLstar};
  const auto URs = std::vector<Real>{hRstar, huRstar, hvRstar};

  // HLLE flux for fallback and optional blend
  auto hlle = [&](const std::vector<Real> & A,
                  const std::vector<Real> & B,
                  const std::vector<Real> & FA,
                  const std::vector<Real> & FB,
                  Real SLv,
                  Real SRv)
  {
    std::vector<Real> f(3, 0.0);
    const Real inv = 1.0 / std::max(SRv - SLv, _degeneracy_eps);
    f[0] = (SRv * FA[0] - SLv * FB[0] + SLv * SRv * (B[0] - A[0])) * inv;
    f[1] = (SRv * FA[1] - SLv * FB[1] + SLv * SRv * (B[1] - A[1])) * inv;
    f[2] = (SRv * FA[2] - SLv * FB[2] + SLv * SRv * (B[2] - A[2])) * inv;
    return f;
  };

  const bool bad = (SR <= SL) || (hLstar <= 0.0 || hRstar <= 0.0) || !std::isfinite(Sstar);
  std::vector<Real> fHLLC(3, 0.0);
  std::vector<Real> fHLLE(3, 0.0);

  fHLLE = hlle(UL, UR, FL, FR, SL, SR);

  if (bad)
  {
    fHLLC = fHLLE;
    ++_fallback_count;
  }
  else if (0.0 <= SL)
    fHLLC = FL;
  else if (SL <= 0.0 && 0.0 <= Sstar)
  {
    fHLLC[0] = FL[0] + SL * (ULs[0] - UL[0]);
    fHLLC[1] = FL[1] + SL * (ULs[1] - UL[1]);
    fHLLC[2] = FL[2] + SL * (ULs[2] - UL[2]);
  }
  else if (Sstar <= 0.0 && 0.0 <= SR)
  {
    fHLLC[0] = FR[0] + SR * (URs[0] - UR[0]);
    fHLLC[1] = FR[1] + SR * (URs[1] - UR[1]);
    fHLLC[2] = FR[2] + SR * (URs[2] - UR[2]);
  }
  else
    fHLLC = FR;

  // Optional compression-based blend to HLLE near strong compression (carbuncle control)
  Real theta = 0.0;
  if (_blend_alpha > 0.0 && SL < 0.0 && SR > 0.0)
  {
    const Real chi = std::abs(unR - unL) / std::max(cL + cR, 1e-12);
    theta = std::max(0.0, std::min(1.0, _blend_alpha * chi));
    if (theta > 0.0)
      ++_blend_count;
  }
  flux.resize(3);
  for (unsigned i = 0; i < 3; ++i)
    flux[i] = (1.0 - theta) * fHLLC[i] + theta * fHLLE[i];
}

void
SWENumericalFluxHLLC::calcJacobian(unsigned int /*iside*/,
                                   dof_id_type /*ielem*/,
                                   dof_id_type /*ineig*/,
                                   const std::vector<Real> & uvec1,
                                   const std::vector<Real> & uvec2,
                                   const RealVectorValue & n,
                                   DenseMatrix<Real> & jac1,
                                   DenseMatrix<Real> & jac2) const
{
  // Same robust approximation you already use: physical dF + 0.5*smax*I on each side.
  jac1.resize(3, 3);
  jac2.resize(3, 3);
  jac1.zero();
  jac2.zero();

  const Real nx = n(0), ny = n(1);

  auto fill_dF = [&](const std::vector<Real> & U, DenseMatrix<Real> & J)
  {
    const Real h = std::max(U[0], 0.0);
    const Real hu = (h > _h_eps) ? U[1] : 0.0;
    const Real hv = (h > _h_eps) ? U[2] : 0.0;
    const Real invh = (h > _h_eps) ? 1.0 / h : 0.0;
    const Real un = (h > _h_eps) ? (hu * nx + hv * ny) * invh : 0.0;
    J(0, 0) = 0.0;
    J(0, 1) = nx;
    J(0, 2) = ny;
    if (h > _h_eps)
    {
      const Real qn = hu * nx + hv * ny;
      const Real d_un_dh = -qn * invh * invh;
      const Real d_un_dhu = nx * invh;
      const Real d_un_dhv = ny * invh;
      J(1, 0) = hu * d_un_dh + _g * h * nx;
      J(1, 1) = un + hu * d_un_dhu;
      J(1, 2) = hu * d_un_dhv;
      J(2, 0) = hv * d_un_dh + _g * h * ny;
      J(2, 1) = hv * d_un_dhu;
      J(2, 2) = un + hv * d_un_dhv;
    }
  };

  fill_dF(uvec1, jac1);
  fill_dF(uvec2, jac2);

  // smax term
  const Real hL = std::max(uvec1[0], 0.0), hR = std::max(uvec2[0], 0.0);
  const Real huL = (hL > _h_eps) ? uvec1[1] : 0.0, hvL = (hL > _h_eps) ? uvec1[2] : 0.0;
  const Real huR = (hR > _h_eps) ? uvec2[1] : 0.0, hvR = (hR > _h_eps) ? uvec2[2] : 0.0;
  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real smax =
      std::max(std::fabs(unL) + std::sqrt(_g * hL), std::fabs(unR) + std::sqrt(_g * hR));
  for (unsigned i = 0; i < 3; ++i)
  {
    jac1(i, i) += 0.5 * smax;
    jac2(i, i) += 0.5 * smax;
  }
}

void
SWENumericalFluxHLLC::initialize()
{
  InternalSideFluxBase::initialize();
  _fallback_count = 0;
  _blend_count = 0;
}

void
SWENumericalFluxHLLC::finalize()
{
  if (_log_debug)
    mooseInfoRepeated("HLLC step stats: HLLE fallback faces = ",
                      _fallback_count,
                      ", blended faces = ",
                      _blend_count);
  InternalSideFluxBase::finalize();
}
