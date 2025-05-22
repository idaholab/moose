//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFluxGasMixDGKernel.h"
#include "NumericalFluxGasMixBase.h"
#include "MooseVariable.h"
#include "THMIndicesGasMix.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", NumericalFluxGasMixDGKernel);

InputParameters
NumericalFluxGasMixDGKernel::validParams()
{
  InputParameters params = ADDGKernel::validParams();

  params.addClassDescription("Adds side fluxes from NumericalFluxGasMixBase objects.");

  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("xirhoA", "Conserved variable xi*rho*A");
  params.addRequiredCoupledVar("rhoA", "Conserved variable rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable rho*E*A");

  params.addRequiredParam<UserObjectName>("numerical_flux",
                                          "Name of NumericalFluxGasMixBase object");

  return params;
}

NumericalFluxGasMixDGKernel::NumericalFluxGasMixDGKernel(const InputParameters & parameters)
  : ADDGKernel(parameters),

    _A_elem(adCoupledValue("A_linear")),
    _A_neig(adCoupledNeighborValue("A_linear")),
    _xirhoA1(getADMaterialProperty<Real>(THM::XIRHOA)),
    _rhoA1(getADMaterialProperty<Real>(THM::RHOA)),
    _rhouA1(getADMaterialProperty<Real>(THM::RHOUA)),
    _rhoEA1(getADMaterialProperty<Real>(THM::RHOEA)),
    _xirhoA2(getNeighborADMaterialProperty<Real>(THM::XIRHOA)),
    _rhoA2(getNeighborADMaterialProperty<Real>(THM::RHOA)),
    _rhouA2(getNeighborADMaterialProperty<Real>(THM::RHOUA)),
    _rhoEA2(getNeighborADMaterialProperty<Real>(THM::RHOEA)),
    _numerical_flux(getUserObject<NumericalFluxGasMixBase>("numerical_flux")),
    _xirhoA_var(coupled("xirhoA")),
    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number()))
{
}

ADReal
NumericalFluxGasMixDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  // construct the left and right solution vectors from the reconstructed solution
  std::vector<ADReal> U1(THMGasMix1D::N_FLUX_INPUTS, 0.0);
  U1[THMGasMix1D::XIRHOA] = _xirhoA1[_qp];
  U1[THMGasMix1D::RHOA] = _rhoA1[_qp];
  U1[THMGasMix1D::RHOUA] = _rhouA1[_qp];
  U1[THMGasMix1D::RHOEA] = _rhoEA1[_qp];
  U1[THMGasMix1D::AREA] = _A_elem[_qp];

  std::vector<ADReal> U2(THMGasMix1D::N_FLUX_INPUTS, 0.0);
  U2[THMGasMix1D::XIRHOA] = _xirhoA2[_qp];
  U2[THMGasMix1D::RHOA] = _rhoA2[_qp];
  U2[THMGasMix1D::RHOUA] = _rhouA2[_qp];
  U2[THMGasMix1D::RHOEA] = _rhoEA2[_qp];
  U2[THMGasMix1D::AREA] = _A_neig[_qp];

  const Real nLR_dot_d = _current_side * 2 - 1.0;

  const std::vector<ADReal> & flux_elem =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), true, U1, U2, nLR_dot_d);
  const std::vector<ADReal> & flux_neig =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), false, U1, U2, nLR_dot_d);

  ADReal re = 0.0;
  switch (type)
  {
    case Moose::Element:
      re = flux_elem[_equation_index] * _test[_i][_qp];
      break;
    case Moose::Neighbor:
      re = -flux_neig[_equation_index] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

std::map<unsigned int, unsigned int>
NumericalFluxGasMixDGKernel::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_xirhoA_var, THMGasMix1D::SPECIES));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMGasMix1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMGasMix1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMGasMix1D::ENERGY));

  return jmap;
}
