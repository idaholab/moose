//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Rogers-Stallybrass-Clements version of effective saturation of water phase.
//  valid for residual saturations = 0, and viscosityOil = 2*viscosityWater.  (the "2" is important
//  here!).
// C Rogers, MP Stallybrass and DL Clements "On two phase filtration under gravity and with boundary
// infiltration: application of a Backlund transformation" Nonlinear Analysis Theory Methods and
// Applications 7 (1983) 785--799.
//
#include "RichardsSeff2waterRSC.h"

registerMooseObject("RichardsApp", RichardsSeff2waterRSC);

InputParameters
RichardsSeff2waterRSC::validParams()
{
  InputParameters params = RichardsSeff::validParams();
  params.addRequiredParam<Real>(
      "oil_viscosity",
      "Viscosity of oil (gas) phase.  It is assumed this is double the water-phase viscosity");
  params.addRequiredParam<Real>("scale_ratio",
                                "This is porosity/permeability/beta^2, where beta "
                                "may be chosen by the user (RSC define beta<0, but "
                                "MOOSE only uses beta^2, so its sign is "
                                "irrelevant).  It has dimensions [time]");
  params.addRequiredParam<Real>("shift", "effective saturation is a function of (Pc - shift)");
  params.addClassDescription("Rogers-Stallybrass-Clements version of effective saturation for the "
                             "water phase, valid for residual saturations = 0, and viscosityOil = "
                             "2*viscosityWater.  seff_water = 1/Sqrt(1 + Exp(Pc - shift)/scale)), "
                             "where scale = 0.25*scale_ratio*oil_viscosity");
  return params;
}

RichardsSeff2waterRSC::RichardsSeff2waterRSC(const InputParameters & parameters)
  : RichardsSeff(parameters),
    _oil_viscosity(getParam<Real>("oil_viscosity")),
    _scale_ratio(getParam<Real>("scale_ratio")),
    _shift(getParam<Real>("shift")),
    _scale(0.25 * _scale_ratio * _oil_viscosity)
{
}

Real
RichardsSeff2waterRSC::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  Real pc = (*p[1])[qp] - (*p[0])[qp];
  return RichardsSeffRSC::seff(pc, _shift, _scale);
}

void
RichardsSeff2waterRSC::dseff(std::vector<const VariableValue *> p,
                             unsigned int qp,
                             std::vector<Real> & result) const
{
  Real pc = (*p[1])[qp] - (*p[0])[qp];
  result[1] = RichardsSeffRSC::dseff(pc, _shift, _scale);
  result[0] = -result[1];
}

void
RichardsSeff2waterRSC::d2seff(std::vector<const VariableValue *> p,
                              unsigned int qp,
                              std::vector<std::vector<Real>> & result) const
{
  Real pc = (*p[1])[qp] - (*p[0])[qp];
  result[1][1] = RichardsSeffRSC::d2seff(pc, _shift, _scale);
  result[0][1] = -result[1][1];
  result[1][0] = -result[1][1];
  result[0][0] = result[1][1];
}
