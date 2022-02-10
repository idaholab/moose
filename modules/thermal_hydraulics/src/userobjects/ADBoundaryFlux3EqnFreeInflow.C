//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnFreeInflow.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnFreeInflow);

InputParameters
ADBoundaryFlux3EqnFreeInflow::validParams()
{
  InputParameters params = ADBoundaryFluxBase::validParams();

  params.addClassDescription("Computes the inflow boundary flux directly for the 1-D, 1-phase, "
                             "variable-area Euler equations");

  params.addRequiredParam<Real>("rho_infinity", "Far-stream density value");
  params.addRequiredParam<Real>("vel_infinity", "Far-stream velocity value");
  params.addRequiredParam<Real>("p_infinity", "Far-stream pressure value");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

ADBoundaryFlux3EqnFreeInflow::ADBoundaryFlux3EqnFreeInflow(const InputParameters & parameters)
  : ADBoundaryFluxBase(parameters),

    _rho_inf(getParam<Real>("rho_infinity")),
    _vel_inf(getParam<Real>("vel_infinity")),
    _p_inf(getParam<Real>("p_infinity")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADBoundaryFlux3EqnFreeInflow::calcFlux(unsigned int /*iside*/,
                                       dof_id_type /*ielem*/,
                                       const std::vector<ADReal> & U1,
                                       const RealVectorValue & /*normal*/,
                                       std::vector<ADReal> & flux) const
{
  const ADReal A = U1[THM3Eqn::CONS_VAR_AREA];

  const ADReal e_inf = _fp.e_from_p_rho(_p_inf, _rho_inf);
  const ADReal E_inf = e_inf + 0.5 * _vel_inf * _vel_inf;

  flux.resize(THM3Eqn::N_EQ);
  flux[THM3Eqn::EQ_MASS] = _rho_inf * _vel_inf * A;
  flux[THM3Eqn::EQ_MOMENTUM] = (_rho_inf * _vel_inf * _vel_inf + _p_inf) * A;
  flux[THM3Eqn::EQ_ENERGY] = _vel_inf * (_rho_inf * E_inf + _p_inf) * A;
}
