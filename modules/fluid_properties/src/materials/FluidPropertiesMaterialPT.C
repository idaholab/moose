//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesMaterialPT.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesMaterialPT);

InputParameters
FluidPropertiesMaterialPT::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("temperature", "Fluid temperature (K)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Fluid properties using the (pressure, temperature) formulation");

  // Restrict properties created
  params.addParam<bool>("compute_entropy", true, "Whether to compute the entropy");
  params.addParam<bool>("compute_sound_speed", true, "Whether to compute the speed of sound");

  return params;
}

FluidPropertiesMaterialPT::FluidPropertiesMaterialPT(const InputParameters & parameters)
  : Material(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),

    _rho(declareProperty<Real>("density")),
    _mu(declareProperty<Real>("viscosity")),
    _cp(declareProperty<Real>("cp")),
    _cv(declareProperty<Real>("cv")),
    _k(declareProperty<Real>("k")),
    _h(declareProperty<Real>("h")),
    _e(declareProperty<Real>("e")),

    _compute_s(getParam<bool>("compute_entropy")),
    _compute_c(getParam<bool>("compute_sound_speed")),

    _s(_compute_s ? &declareProperty<Real>("s") : nullptr),
    _c(_compute_c ? &declareProperty<Real>("c") : nullptr),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterialPT::~FluidPropertiesMaterialPT() {}

void
FluidPropertiesMaterialPT::computeQpProperties()
{
  _rho[_qp] = _fp.rho_from_p_T(_pressure[_qp], _temperature[_qp]);
  _mu[_qp] = _fp.mu_from_p_T(_pressure[_qp], _temperature[_qp]);
  _cp[_qp] = _fp.cp_from_p_T(_pressure[_qp], _temperature[_qp]);
  _cv[_qp] = _fp.cv_from_p_T(_pressure[_qp], _temperature[_qp]);
  _k[_qp] = _fp.k_from_p_T(_pressure[_qp], _temperature[_qp]);
  _h[_qp] = _fp.h_from_p_T(_pressure[_qp], _temperature[_qp]);
  _e[_qp] = _fp.e_from_p_T(_pressure[_qp], _temperature[_qp]);
  if (_compute_s)
    (*_s)[_qp] = _fp.s_from_p_T(_pressure[_qp], _temperature[_qp]);
  if (_compute_c)
    (*_c)[_qp] = _fp.c_from_p_T(_pressure[_qp], _temperature[_qp]);
}
