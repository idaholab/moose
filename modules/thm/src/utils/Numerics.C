#include "Numerics.h"

Real
Reynolds(Real volume_fraction, Real rho, Real v, Real Dh, Real visc)
{
  return volume_fraction * rho * std::fabs(v) * Dh / visc;
}
