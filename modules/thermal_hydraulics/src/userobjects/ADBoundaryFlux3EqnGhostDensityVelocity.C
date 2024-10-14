//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostDensityVelocity);

InputParameters
ADBoundaryFlux3EqnGhostDensityVelocity::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Computes boundary flux from density and velocity for the 3-equation "
                             "model using a ghost cell approach.");

  params.addRequiredParam<Real>("rho", "Density");
  params.addRequiredParam<Real>("vel", "Velocity");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "1-phase fluid properties user object name");

  params.declareControllable("rho vel");

  return params;
}

ADBoundaryFlux3EqnGhostDensityVelocity::ADBoundaryFlux3EqnGhostDensityVelocity(
    const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),

    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _reversible(getParam<bool>("reversible")),

    _fp(getUserObjectByName<SinglePhaseFluidProperties>(
        getParam<UserObjectName>("fluid_properties")))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostDensityVelocity::getGhostCellSolution(
    const std::vector<ADReal> & U_interior) const
{
  const ADReal rhoA = U_interior[THMVACE1D::RHOA];
  const ADReal rhouA = U_interior[THMVACE1D::RHOUA];
  const ADReal rhoEA = U_interior[THMVACE1D::RHOEA];
  const ADReal A = U_interior[THMVACE1D::AREA];

  std::vector<ADReal> U_ghost(THMVACE1D::N_FLUX_INPUTS);
  if (!_reversible || THM::isInlet(_vel, _normal))
  {
    // Get the pressure from the interior solution

    const ADReal rho = rhoA / A;
    const ADReal vel = rhouA / rhoA;
    const ADReal E = rhoEA / rhoA;
    const ADReal e = E - 0.5 * vel * vel;
    const ADReal p = _fp.p_from_v_e(1.0 / rho, e);

    // Compute remaining boundary quantities

    const ADReal e_b = _fp.e_from_p_rho(p, _rho);
    const ADReal E_b = e_b + 0.5 * _vel * _vel;

    // compute ghost solution
    U_ghost[THMVACE1D::RHOA] = _rho * A;
    U_ghost[THMVACE1D::RHOUA] = _rho * _vel * A;
    U_ghost[THMVACE1D::RHOEA] = _rho * E_b * A;
    U_ghost[THMVACE1D::AREA] = A;
  }
  else
  {
    U_ghost[THMVACE1D::RHOA] = rhoA;
    U_ghost[THMVACE1D::RHOUA] = rhoA * _vel;
    U_ghost[THMVACE1D::RHOEA] = rhoEA;
    U_ghost[THMVACE1D::AREA] = A;
  }

  return U_ghost;
}
