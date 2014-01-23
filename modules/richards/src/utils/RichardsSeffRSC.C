//  Rogers-Stallybrass-Clements version of effective saturation as a function of CAPILLARY pressure.
//  valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important here!).
// C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and Applications 7 (1983) 785--799.
//
#include "RichardsSeffRSC.h"

RichardsSeffRSC::RichardsSeffRSC()
{}

Real
RichardsSeffRSC::seff(Real pc, Real shift, Real scale)
{
  Real x = (pc - shift)/scale;
  return 1 - 1/std::pow(1 + std::exp(x), 0.5);
}

Real
RichardsSeffRSC::dseff(Real pc, Real shift, Real scale)
{
  Real x = (pc - shift)/scale;
  Real ex = std::exp(x);
  return 0.5*ex/std::pow(1 + ex, 1.5)/scale;
}

Real
RichardsSeffRSC::d2seff(Real pc, Real shift, Real scale)
{
  Real x = (pc - shift)/scale;
  Real ex = std::exp(x);
  return (-0.75*ex*ex/std::pow(1 + ex, 2.5) + 0.5*ex/std::pow(1 + ex, 1.5))/scale/scale;
}
