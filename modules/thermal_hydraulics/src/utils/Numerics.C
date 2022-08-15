//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Numerics.h"
#include "MooseUtils.h"
#include "ADReal.h"

namespace THM
{

bool
absoluteFuzzyEqualVectors(const RealVectorValue & a, const RealVectorValue & b, const Real & tol)
{
  return MooseUtils::absoluteFuzzyEqual(a(0), b(0), tol) &&
         MooseUtils::absoluteFuzzyEqual(a(1), b(1), tol) &&
         MooseUtils::absoluteFuzzyEqual(a(2), b(2), tol);
}

bool
areParallelVectors(const RealVectorValue & a, const RealVectorValue & b, const Real & tol)
{
  const RealVectorValue c = a.cross(b);
  return absoluteFuzzyEqualVectors(c, RealVectorValue(0, 0, 0), tol);
}

bool
haveSameDirection(const RealVectorValue & a, const RealVectorValue & b, const Real & tol)
{
  return areParallelVectors(a, b, tol) && a * b > 0;
}

Real
applyQuotientRule(const Real & num, const Real & den, const Real & dnum_dy, const Real & dden_dy)
{
  return (dnum_dy * den - num * dden_dy) / (den * den);
}

DenseVector<Real>
applyQuotientRule(const Real & num,
                  const Real & den,
                  const DenseVector<Real> & dnum_dy,
                  const DenseVector<Real> & dden_dy)
{
  DenseVector<Real> d_dy = dnum_dy;
  d_dy *= 1.0 / den;
  d_dy.add(-num / std::pow(den, 2), dden_dy);

  return d_dy;
}

Real
Reynolds(Real volume_fraction, Real rho, Real vel, Real D_h, Real mu)
{
  return volume_fraction * rho * std::fabs(vel) * D_h / mu;
}

ADReal
Reynolds(ADReal volume_fraction, ADReal rho, ADReal vel, ADReal D_h, ADReal mu)
{
  return volume_fraction * rho * std::fabs(vel) * D_h / mu;
}

Real
Prandtl(Real cp, Real mu, Real k)
{
  return cp * mu / k;
}

ADReal
Prandtl(ADReal cp, ADReal mu, ADReal k)
{
  return cp * mu / k;
}

Real
Grashof(Real beta, Real dT, Real D_h, Real rho_liquid, Real mu_liquid, Real gravity_magnitude)
{
  return gravity_magnitude * beta * dT * std::pow(D_h, 3) * (rho_liquid * rho_liquid) /
         (mu_liquid * mu_liquid);
}

ADReal
Grashof(
    ADReal beta, ADReal dT, ADReal D_h, ADReal rho_liquid, ADReal mu_liquid, Real gravity_magnitude)
{
  return gravity_magnitude * beta * dT * std::pow(D_h, 3) * (rho_liquid * rho_liquid) /
         (mu_liquid * mu_liquid);
}

Real
Laplace(Real surf_tension, Real delta_rho, Real gravity_magnitude)
{
  return std::sqrt(surf_tension / (gravity_magnitude * delta_rho));
}

ADReal
Laplace(ADReal surf_tension, ADReal delta_rho, Real gravity_magnitude)
{
  return std::sqrt(surf_tension / (gravity_magnitude * delta_rho));
}

Real
viscosityNumber(
    Real viscosity, Real surf_tension, Real rho_k, Real delta_rho, Real gravity_magnitude)
{
  return viscosity /
         std::sqrt(rho_k * surf_tension * std::sqrt(surf_tension / gravity_magnitude / delta_rho));
}

ADReal
viscosityNumber(
    ADReal viscosity, ADReal surf_tension, ADReal rho_k, ADReal delta_rho, Real gravity_magnitude)
{
  return viscosity /
         std::sqrt(rho_k * surf_tension * std::sqrt(surf_tension / gravity_magnitude / delta_rho));
}

Real
wallHeatTransferCoefficient(Real Nu, Real k, Real D_h)
{
  return Nu * k / D_h;
}

ADReal
wallHeatTransferCoefficient(ADReal Nu, ADReal k, ADReal D_h)
{
  return Nu * k / D_h;
}

Real
Dean(Real Re, Real doD)
{
  return Re * std::sqrt(doD);
}

ADReal
Dean(ADReal Re, ADReal doD)
{
  return Re * std::sqrt(doD);
}

void
vel_from_arhoA_arhouA(Real arhoA, Real arhouA, Real & vel, Real & dvel_darhoA, Real & dvel_darhouA)
{
  vel = arhouA / arhoA;
  dvel_darhoA = -arhouA / (arhoA * arhoA);
  dvel_darhouA = 1.0 / arhoA;
}

ADReal
vel_from_arhoA_arhouA(ADReal arhoA, ADReal arhouA)
{
  return arhouA / arhoA;
}

Real
dvel_darhoA(Real arhoA, Real arhouA)
{
  return -arhouA / (arhoA * arhoA);
}

Real
dvel_darhouA(Real arhoA)
{
  return 1.0 / arhoA;
}

void
rho_from_arhoA_alpha_A(
    Real arhoA, Real alpha, Real A, Real & rho, Real & drho_darhoA, Real & drho_dalpha)
{
  rho = arhoA / (alpha * A);

  drho_darhoA = 1.0 / (alpha * A);
  drho_dalpha = -arhoA / (alpha * alpha * A);
}

ADReal
rho_from_arhoA_alpha_A(ADReal arhoA, ADReal alpha, ADReal A)
{
  return arhoA / (alpha * A);
}

void
v_from_rhoA_A(Real rhoA, Real A, Real & v, Real & dv_drhoA)
{
  v = A / rhoA;
  dv_drhoA = -A / (rhoA * rhoA);
}

ADReal
v_from_rhoA_A(ADReal rhoA, ADReal A)
{
  return A / rhoA;
}

void
v_from_arhoA_alpha_A(Real arhoA, Real alpha, Real A, Real & v, Real & dv_darhoA, Real & dv_dalpha)
{
  v = (alpha * A) / arhoA;
  dv_darhoA = -(alpha * A) / (arhoA * arhoA);
  dv_dalpha = A / arhoA;
}

ADReal
v_from_arhoA_alpha_A(ADReal arhoA, ADReal alpha, ADReal A)
{
  return (alpha * A) / arhoA;
}

void
v_from_rho(Real rho, Real & v, Real & dv_drho)
{
  v = 1.0 / rho;
  dv_drho = -1.0 / (rho * rho);
}

Real
dv_dalpha_liquid(Real area, Real arhoA, bool is_liquid)
{
  const Real sign = is_liquid ? 1.0 : -1.0;
  return sign * (area / arhoA);
}

Real
dv_darhoA(Real area, Real arhoA)
{
  return -area / arhoA / arhoA;
}

void
e_from_arhoA_arhouA_arhoEA(Real arhoA,
                           Real arhouA,
                           Real arhoEA,
                           Real & e,
                           Real & de_darhoA_val,
                           Real & de_darhouA_val,
                           Real & de_darhoEA_val)
{
  e = arhoEA / arhoA - 0.5 * arhouA * arhouA / (arhoA * arhoA);
  de_darhoA_val = de_darhoA(arhoA, arhouA, arhoEA);
  de_darhouA_val = de_darhouA(arhoA, arhouA);
  de_darhoEA_val = de_darhoEA(arhoA);
}

ADReal
e_from_arhoA_arhouA_arhoEA(ADReal arhoA, ADReal arhouA, ADReal arhoEA)
{
  return arhoEA / arhoA - 0.5 * arhouA * arhouA / (arhoA * arhoA);
}

void
e_from_E_vel(Real E, Real vel, Real & e, Real & de_dE, Real & de_dvel)
{
  e = E - 0.5 * vel * vel;
  de_dE = 1;
  de_dvel = -vel;
}
ADReal
e_from_E_vel(ADReal E, ADReal vel)
{
  return E - 0.5 * vel * vel;
}

Real
de_darhoA(Real arhoA, Real arhouA, Real arhoEA)
{
  return (-arhoEA / arhoA / arhoA + arhouA * arhouA / arhoA / arhoA / arhoA);
}

Real
de_darhouA(Real arhoA, Real arhouA)
{
  return (-arhouA / arhoA / arhoA);
}

Real
de_darhoEA(Real arhoA)
{
  return (1 / arhoA);
}

void
E_from_arhoA_arhoEA(Real arhoA, Real arhoEA, Real & E, Real & dE_darhoA, Real & dE_darhoEA)
{
  E = arhoEA / arhoA;
  dE_darhoA = -arhoEA / (arhoA * arhoA);
  dE_darhoEA = 1.0 / arhoA;
}
ADReal
E_from_arhoA_arhoEA(ADReal arhoA, ADReal arhoEA)
{
  return arhoEA / arhoA;
}

void
E_from_e_vel(Real e, Real vel, Real & E, Real & dE_de, Real & dE_dvel)
{
  E = e + 0.5 * vel * vel;
  dE_de = 1.0;
  dE_dvel = vel;
}

void
h_from_e_p_rho(Real e, Real p, Real rho, Real & h, Real & dh_de, Real & dh_dp, Real & dh_drho)
{
  h = e + p / rho;
  dh_de = 1.0;
  dh_dp = 1.0 / rho;
  dh_drho = -p / (rho * rho);
}

ADReal
h_from_e_p_rho(ADReal e, ADReal p, ADReal rho)
{
  return e + p / rho;
}

bool
isInlet(Real vel, Real normal)
{
  return (vel * normal) < 0;
}

bool
isInlet(ADReal vel, Real normal)
{
  return (MetaPhysicL::raw_value(vel) * normal) < 0;
}

bool
isOutlet(Real vel, Real normal)
{
  return (vel * normal) >= 0;
}

bool
isOutlet(ADReal vel, Real normal)
{
  return (MetaPhysicL::raw_value(vel) * normal) >= 0;
}

} // namespace THM
