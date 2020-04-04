//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Rogers-Stallybrass-Clements version of effective saturation of water phase as a function of
//  pressure, and derivatives wrt that pressure.
//  This is mostly useful for 2phase, not single phase, models.
//  valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important
//  here!).
// C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary
// infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and
// Applications 7 (1983) 785--799.
//
#include "RichardsSeff1RSC.h"

registerMooseObject("RichardsApp", RichardsSeff1RSC);

InputParameters
RichardsSeff1RSC::validParams()
{
  InputParameters params = RichardsSeff::validParams();
  params.addParam<Real>("oil_viscosity",
                        "Viscosity of oil (gas) phase.  It is assumed this is "
                        "double the water-phase viscosity.  (Note that this "
                        "effective saturation is mostly useful for 2-phase, not "
                        "single-phase.)");
  params.addParam<Real>("scale_ratio",
                        "This is porosity/permeability/beta^2, where beta may be "
                        "chosen by the user.  It has dimensions [time]");
  params.addParam<Real>("shift", "effective saturation is a function of (Pc - shift)");
  params.addClassDescription(
      "Rogers-Stallybrass-Clements version of effective saturation for the water phase, valid for "
      "residual saturations = 0, and viscosityOil = 2*viscosityWater.  seff_water = 1/Sqrt(1 + "
      "Exp((Pc - shift)/scale)), where scale = 0.25*scale_ratio*oil_viscosity.  Note that this "
      "effective saturation is mostly useful for 2-phase, not single-phase.");
  return params;
}

RichardsSeff1RSC::RichardsSeff1RSC(const InputParameters & parameters)
  : RichardsSeff(parameters),
    _oil_viscosity(getParam<Real>("oil_viscosity")),
    _scale_ratio(getParam<Real>("scale_ratio")),
    _shift(getParam<Real>("shift")),
    _scale(0.25 * _scale_ratio * _oil_viscosity)
{
}

Real
RichardsSeff1RSC::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  Real pc = -(*p[0])[qp];
  return RichardsSeffRSC::seff(pc, _shift, _scale);
}

void
RichardsSeff1RSC::dseff(std::vector<const VariableValue *> p,
                        unsigned int qp,
                        std::vector<Real> & result) const
{
  Real pc = -(*p[0])[qp];
  result[0] = -RichardsSeffRSC::dseff(pc, _shift, _scale);
}

void
RichardsSeff1RSC::d2seff(std::vector<const VariableValue *> p,
                         unsigned int qp,
                         std::vector<std::vector<Real>> & result) const
{
  Real pc = -(*p[0])[qp];
  result[0][0] = RichardsSeffRSC::d2seff(pc, _shift, _scale);
}
