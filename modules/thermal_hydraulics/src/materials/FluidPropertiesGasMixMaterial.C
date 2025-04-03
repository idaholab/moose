//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesGasMixMaterial.h"
#include "VaporMixtureFluidProperties.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", FluidPropertiesGasMixMaterial);

InputParameters
FluidPropertiesGasMixMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("xirhoA", "xi*rho*A variable");
  params.addRequiredCoupledVar("rhoA", "rho*A variable");
  params.addRequiredCoupledVar("rhouA", "rho*u*A variable");
  params.addRequiredCoupledVar("rhoEA", "rho*E*A variable");
  params.addRequiredCoupledVar("area", "Cross-sectional area variable");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The VaporMixtureFluidProperties object");

  params.addClassDescription("Computes various fluid properties for FlowModelGasMix.");

  return params;
}

FluidPropertiesGasMixMaterial::FluidPropertiesGasMixMaterial(const InputParameters & parameters)
  : Material(parameters),

    _A(adCoupledValue("area")),
    _xirhoA(adCoupledValue("xirhoA")),
    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

    _xi(declareADProperty<Real>(THM::MASS_FRACTION)),
    _rho(declareADProperty<Real>(THM::DENSITY)),
    _v(declareADProperty<Real>(THM::SPECIFIC_VOLUME)),
    _vel(declareADProperty<Real>(THM::VELOCITY)),
    _e(declareADProperty<Real>(THM::SPECIFIC_INTERNAL_ENERGY)),
    _p(declareADProperty<Real>(THM::PRESSURE)),
    _T(declareADProperty<Real>(THM::TEMPERATURE)),
    _h(declareADProperty<Real>(THM::SPECIFIC_ENTHALPY)),
    _H(declareADProperty<Real>(THM::SPECIFIC_TOTAL_ENTHALPY)),
    _c(declareADProperty<Real>(THM::SOUND_SPEED)),
    _cp(declareADProperty<Real>(THM::SPECIFIC_HEAT_CONSTANT_PRESSURE)),
    _cv(declareADProperty<Real>(THM::SPECIFIC_HEAT_CONSTANT_VOLUME)),
    _k(declareADProperty<Real>(THM::THERMAL_CONDUCTIVITY)),
    _mu(declareADProperty<Real>(THM::DYNAMIC_VISCOSITY)),

    _fp(getUserObject<VaporMixtureFluidProperties>("fluid_properties"))
{
}

void
FluidPropertiesGasMixMaterial::computeQpProperties()
{
  _xi[_qp] = _xirhoA[_qp] / _rhoA[_qp];
  _rho[_qp] = _rhoA[_qp] / _A[_qp];
  _v[_qp] = 1.0 / _rho[_qp];
  _vel[_qp] = _rhouA[_qp] / _rhoA[_qp];
  _e[_qp] = (_rhoEA[_qp] - 0.5 * _rhouA[_qp] * _rhouA[_qp] / _rhoA[_qp]) / _rhoA[_qp];
  _p[_qp] = _fp.p_from_v_e(_v[_qp], _e[_qp], {_xi[_qp]});
  _T[_qp] = _fp.T_from_v_e(_v[_qp], _e[_qp], {_xi[_qp]});
  _h[_qp] = _e[_qp] + _p[_qp] / _rho[_qp];
  _H[_qp] = _h[_qp] + 0.5 * _vel[_qp] * _vel[_qp];
  _c[_qp] = _fp.c_from_p_T(_p[_qp], _T[_qp], {_xi[_qp]});
  _cp[_qp] = _fp.cp_from_p_T(_p[_qp], _T[_qp], {_xi[_qp]});
  _cv[_qp] = _fp.cv_from_p_T(_p[_qp], _T[_qp], {_xi[_qp]});
  _k[_qp] = _fp.k_from_p_T(_p[_qp], _T[_qp], {_xi[_qp]});
  _mu[_qp] = _fp.mu_from_p_T(_p[_qp], _T[_qp], {_xi[_qp]});
}
