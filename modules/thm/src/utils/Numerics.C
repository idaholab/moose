#include "Numerics.h"

namespace THM
{

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
Reynolds(Real volume_fraction, Real rho, Real v, Real Dh, Real visc)
{
  return volume_fraction * rho * std::fabs(v) * Dh / visc;
}

Real
Prandtl(Real cp, Real mu, Real k)
{
  return cp * mu / k;
}

Real
Grashof(Real beta, Real dT, Real Dh, Real rho_l, Real visc_l, Real gravity_magnitude)
{
  // Eq. 6-17
  return gravity_magnitude * beta * dT * std::pow(Dh, 3) * (rho_l * rho_l) / (visc_l * visc_l);
}

Real
Laplace(Real surf_tension, Real delta_rho, Real gravity_magnitude)
{
  // Eq. 4-119; 5-13.
  return std::sqrt(surf_tension / (gravity_magnitude * delta_rho));
}

Real
viscosityNumber(
    Real viscosity, Real surf_tension, Real rho_k, Real delta_rho, Real gravity_magnitude)
{
  // Equation (4-23), page 129. See also its definition on page 120.
  return viscosity /
         std::sqrt(rho_k * surf_tension * std::sqrt(surf_tension / gravity_magnitude / delta_rho));
}

Real
wallHeatTransferCoefficient(Real Nu, Real k, Real Dh)
{
  return Nu * k / Dh;
}

void
vel_from_arhoA_arhouA(Real arhoA, Real arhouA, Real & vel, Real & dvel_darhoA, Real & dvel_darhouA)
{
  vel = arhouA / arhoA;
  dvel_darhoA = -arhouA / (arhoA * arhoA);
  dvel_darhouA = 1.0 / arhoA;
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

void
v_from_rhoA_A(Real rhoA, Real A, Real & v, Real & dv_drhoA)
{
  v = A / rhoA;
  dv_drhoA = -A / (rhoA * rhoA);
}

void
v_from_arhoA_alpha_A(Real arhoA, Real alpha, Real A, Real & v, Real & dv_darhoA, Real & dv_dalpha)
{
  v = (alpha * A) / arhoA;
  dv_darhoA = -(alpha * A) / (arhoA * arhoA);
  dv_dalpha = A / arhoA;
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

void
e_from_E_vel(Real E, Real vel, Real & e, Real & de_dE, Real & de_dvel)
{
  e = E - 0.5 * vel * vel;
  de_dE = 1;
  de_dvel = -vel;
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

void
E_from_e_vel(Real e, Real vel, Real & E, Real & dE_de, Real & dE_dvel)
{
  E = e + 0.5 * vel * vel;
  dE_de = 1.0;
  dE_dvel = vel;
}

bool
isInlet(Real vel, Real normal)
{
  return (vel * normal) < 0;
}

bool
isOutlet(Real vel, Real normal)
{
  return (vel * normal) >= 0;
}
} // namespace THM
