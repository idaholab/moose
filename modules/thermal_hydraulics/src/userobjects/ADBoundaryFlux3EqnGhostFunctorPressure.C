//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostFunctorPressure.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostFunctorPressure);

InputParameters
ADBoundaryFlux3EqnGhostFunctorPressure::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Computes boundary flux from a functor pressure for the 1-D, "
                             "1-phase, variable-area Euler equations");

  params.addRequiredParam<MooseFunctorName>("p", "Pressure");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  params.declareControllable("p");
  return params;
}

ADBoundaryFlux3EqnGhostFunctorPressure::ADBoundaryFlux3EqnGhostFunctorPressure(
    const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),
    ADFunctorInterface(this),

    _p(getFunctor<ADReal>("p")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostFunctorPressure::getGhostCellSolution(const std::vector<ADReal> & U) const
{
  const ADReal rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal A = U[THM3Eqn::CONS_VAR_AREA];

  // Both these functors should be constant anyway
  Node a(Point(0, 0, 0), 1);
  Moose::NodeArg node_arg = {&a, 0};
  Moose::StateArg time_arg = {0, Moose::SolutionIterationType::Time};

  const ADReal rho = rhoA / A;
  const ADReal vel = rhouA / rhoA;
  const ADReal E = _fp.e_from_p_rho(_p(node_arg, time_arg), rho) + 0.5 * vel * vel;

  std::vector<ADReal> U_ghost(THM3Eqn::N_CONS_VAR);
  U_ghost[THM3Eqn::CONS_VAR_RHOA] = rhoA;
  U_ghost[THM3Eqn::CONS_VAR_RHOUA] = rhouA;
  U_ghost[THM3Eqn::CONS_VAR_RHOEA] = rhoA * E;
  U_ghost[THM3Eqn::CONS_VAR_AREA] = A;

  return U_ghost;
}
