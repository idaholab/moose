/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow2PhasePS_VG.h"
#include "PorousFlowVanGenuchten.h"

#include <limits>

template <>
InputParameters
validParams<PorousFlow2PhasePS_VG>()
{
  InputParameters params = validParams<PorousFlow2PhasePS>();
  params.addRequiredRangeCheckedParam<Real>("m", "m >= 0 & m <= 1", "van Genuchten exponent m");
  params.addRangeCheckedParam<Real>("pc_max",
                                    -std::numeric_limits<Real>::max(),
                                    "pc_max <= 0",
                                    "Maximum capillary pressure (Pa). Must be <= 0. Default is "
                                    "-std::numeric_limits<Real>::max()");
  params.addRequiredRangeCheckedParam<Real>(
      "p0", "p0 > 0", "Capillary pressure coefficient P0. Must be > 0");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations "
                             "in a 2-phase isothermal situation using a van Genucten capillary "
                             "pressure, and derivatives of these with respect to the "
                             "PorousFlowVariables");
  return params;
}

PorousFlow2PhasePS_VG::PorousFlow2PhasePS_VG(const InputParameters & parameters)
  : PorousFlow2PhasePS(parameters),

    _m(getParam<Real>("m")),
    _pc_max(getParam<Real>("pc_max")),
    _p0(getParam<Real>("p0"))
{
  if (_dictator.numPhases() != 2)
    mooseError("The Dictator proclaims that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow2PhasePS_VG can only be used for 2-phase simulation.  Be aware "
               "that the Dictator has noted your mistake.");
}

Real
PorousFlow2PhasePS_VG::capillaryPressure(Real seff) const
{
  return PorousFlowVanGenuchten::capillaryPressure(seff, _m, _p0, _pc_max);
}

Real
PorousFlow2PhasePS_VG::dCapillaryPressure_dS(Real seff) const
{
  return PorousFlowVanGenuchten::dCapillaryPressure(seff, _m, _p0, _pc_max);
}

Real
PorousFlow2PhasePS_VG::d2CapillaryPressure_dS2(Real seff) const
{
  return PorousFlowVanGenuchten::d2CapillaryPressure(seff, _m, _p0, _pc_max);
}
