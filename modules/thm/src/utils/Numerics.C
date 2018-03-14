#include "Numerics.h"

namespace RELAP7
{

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
Grashof(Real beta, Real dT, Real Dh, Real rho_l, Real visc_l)
{
  // Eq. 6-17
  return gravity_const * beta * dT * std::pow(Dh, 3) * (rho_l * rho_l) / (visc_l * visc_l);
}

Real
Laplace(Real surf_tension, Real delta_rho)
{
  // Eq. 4-119; 5-13.
  return std::sqrt(surf_tension / (gravity_const * delta_rho));
}

Real
viscosityNumber(Real viscosity, Real surf_tension, Real rho_k, Real delta_rho)
{
  // Equation (4-23), page 129. See also its definition on page 120.
  return viscosity /
         std::sqrt(rho_k * surf_tension * std::sqrt(surf_tension / gravity_const / delta_rho));
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
}
