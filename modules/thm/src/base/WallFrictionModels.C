#include "WallFrictionModels.h"
#include "Numerics.h"

namespace WallFriction
{

Real
DarcyFrictionFactor(const Real & f_F)
{
  return 4 * f_F;
}

Real
FanningFrictionFactorChurchill(Real Re, Real roughness, Real Dh)
{
  Real Re_limit = std::max(Re, 10.0);

  Real a =
      std::pow(2.457 * std::log(1.0 / (std::pow(7.0 / Re_limit, 0.9) + 0.27 * roughness / Dh)), 16);
  Real b = std::pow(3.753e4 / Re_limit, 16);
  return 2.0 * std::pow(std::pow(8.0 / Re_limit, 12) + 1.0 / std::pow(a + b, 1.5), 1.0 / 12.0);
}
}
