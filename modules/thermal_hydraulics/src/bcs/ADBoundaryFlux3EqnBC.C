//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnBC.h"
#include "MooseVariable.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnBC);

InputParameters
ADBoundaryFlux3EqnBC::validParams()
{
  InputParameters params = BoundaryFlux1PhaseBaseBC::validParams();

  params.addClassDescription(
      "Boundary conditions for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("passives_times_area", "Passive transport solution variables");

  return params;
}

ADBoundaryFlux3EqnBC::ADBoundaryFlux3EqnBC(const InputParameters & parameters)
  : BoundaryFlux1PhaseBaseBC(parameters),

    _passives_times_area(getADMaterialProperty<std::vector<Real>>("passives_times_area")),
    _n_passives(coupledComponents("passives_times_area"))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnBC::fluxInputVector() const
{
  std::vector<ADReal> U(THMVACE1D::N_FLUX_INPUTS + _n_passives, 0);
  U[THMVACE1D::RHOA] = _rhoA[_qp];
  U[THMVACE1D::RHOUA] = _rhouA[_qp];
  U[THMVACE1D::RHOEA] = _rhoEA[_qp];
  U[THMVACE1D::AREA] = _A_linear[_qp];
  for (const auto i : make_range(_n_passives))
    U[THMVACE1D::N_FLUX_INPUTS + i] = _passives_times_area[_qp][i];

  return U;
}

std::map<unsigned int, unsigned int>
ADBoundaryFlux3EqnBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMVACE1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMVACE1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMVACE1D::ENERGY));
  for (const auto i : make_range(_n_passives))
    jmap.insert(std::pair<unsigned int, unsigned int>(coupled("passives_times_area", i),
                                                      THMVACE1D::N_FLUX_OUTPUTS + i));

  return jmap;
}
