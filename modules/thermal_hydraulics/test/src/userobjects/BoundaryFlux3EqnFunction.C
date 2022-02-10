//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFlux3EqnFunction.h"
#include "THMIndices3Eqn.h"
#include "Function.h"
#include "Assembly.h"

registerMooseObject("ThermalHydraulicsTestApp", BoundaryFlux3EqnFunction);

InputParameters
BoundaryFlux3EqnFunction::validParams()
{
  InputParameters params = ADBoundaryFluxBase::validParams();

  params.addClassDescription(
      "Computes the 1-phase boundary flux directly from specified functions.");

  params.addRequiredParam<FunctionName>("rho", "Specified density function");
  params.addRequiredParam<FunctionName>("vel", "Specified velocity function");
  params.addRequiredParam<FunctionName>("p", "Specified pressure function");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

BoundaryFlux3EqnFunction::BoundaryFlux3EqnFunction(const InputParameters & parameters)
  : ADBoundaryFluxBase(parameters),

    _rho_fn(getFunction("rho")),
    _vel_fn(getFunction("vel")),
    _p_fn(getFunction("p")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
BoundaryFlux3EqnFunction::calcFlux(unsigned int /*iside*/,
                                   dof_id_type /*ielem*/,
                                   const std::vector<ADReal> & U1,
                                   const RealVectorValue & /*normal*/,
                                   std::vector<ADReal> & flux) const
{
  const Point & q_point = _assembly.sideElem()->vertex_average();
  const Real rho = _rho_fn.value(_t, q_point);
  const Real vel = _vel_fn.value(_t, q_point);
  const Real p = _p_fn.value(_t, q_point);

  const Real A = MetaPhysicL::raw_value(U1[THM3Eqn::CONS_VAR_AREA]);

  const Real e = _fp.e_from_p_rho(p, rho);
  const Real E = e + 0.5 * vel * vel;

  flux.resize(THM3Eqn::N_EQ);
  flux[THM3Eqn::EQ_MASS] = rho * vel * A;
  flux[THM3Eqn::EQ_MOMENTUM] = (rho * vel * vel + p) * A;
  flux[THM3Eqn::EQ_ENERGY] = vel * (rho * E + p) * A;
}
