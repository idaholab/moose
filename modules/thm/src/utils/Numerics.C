#include "Numerics.h"

extern "C" double SIGMA_TS(double t);

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
  static const Real g = 9.81;            // gravitational acceleration
  return g * beta * dT * std::pow(Dh, 3) * (rho_l * rho_l) / (visc_l * visc_l);
}

Real
wallHeatTransferCoefficient(Real Nu, Real k, Real Dh)
{
  return Nu * k / Dh;
}

Real
surfaceTension(Real temperature)
{
  return SIGMA_TS(temperature) * 1e-3;               // [ N/m]
}
