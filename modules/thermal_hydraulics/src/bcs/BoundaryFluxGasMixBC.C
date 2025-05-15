//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFluxGasMixBC.h"
#include "THMNames.h"
#include "THMIndicesGasMix.h"

registerMooseObject("ThermalHydraulicsApp", BoundaryFluxGasMixBC);

InputParameters
BoundaryFluxGasMixBC::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnBC::validParams();

  params.addClassDescription(
      "Boundary conditions for a FlowChannelGasMix using a boundary flux object.");

  params.addRequiredCoupledVar("xirhoA", "Conserved variable: x*rho*A");

  return params;
}

BoundaryFluxGasMixBC::BoundaryFluxGasMixBC(const InputParameters & parameters)
  : ADBoundaryFlux3EqnBC(parameters),
    _xirhoA(getADMaterialProperty<Real>(THM::XIRHOA)),
    _xirhoA_var(coupled("xirhoA"))
{
}

std::vector<ADReal>
BoundaryFluxGasMixBC::fluxInputVector() const
{
  std::vector<ADReal> U(THMGasMix1D::N_FLUX_INPUTS, 0);
  U[THMGasMix1D::XIRHOA] = _xirhoA[_qp];
  U[THMGasMix1D::RHOA] = _rhoA[_qp];
  U[THMGasMix1D::RHOUA] = _rhouA[_qp];
  U[THMGasMix1D::RHOEA] = _rhoEA[_qp];
  U[THMGasMix1D::AREA] = _A_linear[_qp];

  return U;
}

std::map<unsigned int, unsigned int>
BoundaryFluxGasMixBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_xirhoA_var, THMGasMix1D::SPECIES));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMGasMix1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMGasMix1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMGasMix1D::ENERGY));

  return jmap;
}
