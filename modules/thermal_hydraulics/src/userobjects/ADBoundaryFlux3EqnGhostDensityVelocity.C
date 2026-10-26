//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostDensityVelocity.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include "THMIndicesVACE.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostDensityVelocity);

InputParameters
ADBoundaryFlux3EqnGhostDensityVelocity::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Computes boundary flux from density and velocity for the 3-equation "
                             "model using a ghost cell approach.");

  params.addRequiredParam<FunctionName>("rho", "Function specifying the density");
  params.addRequiredParam<FunctionName>("vel", "Function specifying the velocity");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "1-phase fluid properties user object name");

  return params;
}

ADBoundaryFlux3EqnGhostDensityVelocity::ADBoundaryFlux3EqnGhostDensityVelocity(
    const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),

    _rho_fn(getFunction("rho")),
    _vel_fn(getFunction("vel")),
    _reversible(getParam<bool>("reversible")),

    _fp(getUserObjectByName<SinglePhaseFluidProperties>(
        getParam<UserObjectName>("fluid_properties")))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostDensityVelocity::getGhostCellSolution(const std::vector<ADReal> & U_interior,
                                                             const Point & point) const
{
  const Real rho_b = _rho_fn.value(_t, point);
  const Real vel_b = _vel_fn.value(_t, point);

  mooseAssert(U_interior.size() == THMVACE1D::N_FLUX_INPUTS, "Passive transport not implemented");
  const ADReal rhoA = U_interior[THMVACE1D::RHOA];
  const ADReal rhouA = U_interior[THMVACE1D::RHOUA];
  const ADReal rhoEA = U_interior[THMVACE1D::RHOEA];
  const ADReal A = U_interior[THMVACE1D::AREA];

  std::vector<ADReal> U_ghost(THMVACE1D::N_FLUX_INPUTS);
  if (!_reversible || THM::isInlet(vel_b, _normal))
  {
    // Get the pressure from the interior solution

    const ADReal rho = rhoA / A;
    const ADReal vel = rhouA / rhoA;
    const ADReal E = rhoEA / rhoA;
    const ADReal e = E - 0.5 * vel * vel;
    const ADReal p = _fp.p_from_v_e(1.0 / rho, e);

    // Compute remaining boundary quantities

    const ADReal e_b = _fp.e_from_p_rho(p, rho_b);
    const ADReal E_b = e_b + 0.5 * vel_b * vel_b;

    // compute ghost solution
    U_ghost[THMVACE1D::RHOA] = rho_b * A;
    U_ghost[THMVACE1D::RHOUA] = rho_b * vel_b * A;
    U_ghost[THMVACE1D::RHOEA] = rho_b * E_b * A;
    U_ghost[THMVACE1D::AREA] = A;
  }
  else
  {
    U_ghost[THMVACE1D::RHOA] = rhoA;
    U_ghost[THMVACE1D::RHOUA] = rhoA * vel_b;
    U_ghost[THMVACE1D::RHOEA] = rhoEA;
    U_ghost[THMVACE1D::AREA] = A;
  }

  return U_ghost;
}
