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
surfaceTension(Real temperature)
{
  return SIGMA_TS(temperature) * 1e-3;               // [ N/m]
}
