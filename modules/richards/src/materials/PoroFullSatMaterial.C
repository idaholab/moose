//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PoroFullSatMaterial.h"

registerMooseObject("RichardsApp", PoroFullSatMaterial);

InputParameters
PoroFullSatMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<Real>(
      "porosity0",
      "The porosity of the material when porepressure and volumetric strain are zero.  Eg, 0.1");
  params.addRequiredRangeCheckedParam<Real>("biot_coefficient",
                                            "biot_coefficient>=0 & biot_coefficient<=1",
                                            "The Biot coefficient.  Eg, 0.9");
  params.addRequiredRangeCheckedParam<Real>(
      "solid_bulk_compliance",
      "solid_bulk_compliance>=0",
      "The solid bulk compliance (the reciprocal of the solid bulk modulus)");
  params.addRequiredRangeCheckedParam<Real>(
      "fluid_bulk_compliance",
      "fluid_bulk_compliance>=0",
      "The fluid bulk compliance (the reciprocal of the fluid bulk modulus)");
  params.addRequiredCoupledVar("porepressure", "The porepressure");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<bool>("constant_porosity", false, "Set the porosity equal to porosity0 always");
  params.addClassDescription("This Material is designed to calculate and store all the quantities "
                             "needed for the fluid-flow part of poromechanics, assuming a "
                             "fully-saturated, single-phase fluid with constant bulk modulus");
  return params;
}

PoroFullSatMaterial::PoroFullSatMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),

    _phi0(getParam<Real>("porosity0")),
    _alpha(getParam<Real>("biot_coefficient")),
    _one_over_K(getParam<Real>("solid_bulk_compliance")),
    _one_over_Kf(getParam<Real>("fluid_bulk_compliance")),
    _constant_porosity(getParam<bool>("constant_porosity")),

    _porepressure(coupledValue("porepressure")),
    _porepressure_name(coupledName("porepressure", 0)),

    _ndisp(coupledComponents("displacements")),
    _grad_disp(_ndisp),

    _vol_strain(declareProperty<Real>("volumetric_strain")),

    _biot_coefficient(declareProperty<Real>("biot_coefficient")),

    _porosity(declareProperty<Real>("porosity")),
    _dporosity_dP(declarePropertyDerivative<Real>("porosity", _porepressure_name)),
    _dporosity_dep(declarePropertyDerivative<Real>("porosity", "volumetric_strain")),

    _one_over_biot_modulus(declareProperty<Real>("one_over_biot_modulus")),
    _done_over_biot_modulus_dP(
        declarePropertyDerivative<Real>("one_over_biot_modulus", _porepressure_name)),
    _done_over_biot_modulus_dep(
        declarePropertyDerivative<Real>("one_over_biot_modulus", "volumetric_strain"))
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _grad_disp[i] = &coupledGradient("displacements", i);
}

void
PoroFullSatMaterial::initQpStatefulProperties()
{
  _vol_strain[_qp] = 0.0;
}

void
PoroFullSatMaterial::computeQpProperties()
{
  _biot_coefficient[_qp] = _alpha;

  _vol_strain[_qp] = 0;
  for (unsigned i = 0; i < _ndisp; ++i)
    _vol_strain[_qp] += (*_grad_disp[i])[_qp](i); // cartesian coordinates?

  if (_constant_porosity)
  {
    _porosity[_qp] = _phi0;
    _dporosity_dP[_qp] = 0;
    _dporosity_dep[_qp] = 0;

    _one_over_biot_modulus[_qp] =
        (1 - _alpha) * (_alpha - _porosity[_qp]) * _one_over_K + _porosity[_qp] * _one_over_Kf;
    _done_over_biot_modulus_dP[_qp] = 0;
    _done_over_biot_modulus_dep[_qp] = 0;
  }
  else
  {
    _porosity[_qp] =
        _alpha + (_phi0 - _alpha) *
                     std::exp(-(1 - _alpha) * _one_over_K * _porepressure[_qp] - _vol_strain[_qp]);
    _dporosity_dP[_qp] =
        (_phi0 - _alpha) * (_alpha - 1) * _one_over_K *
        std::exp(-(1 - _alpha) * _one_over_K * _porepressure[_qp] - _vol_strain[_qp]);
    _dporosity_dep[_qp] =
        -(_phi0 - _alpha) *
        std::exp(-(1 - _alpha) * _one_over_K * _porepressure[_qp] - _vol_strain[_qp]);

    _one_over_biot_modulus[_qp] =
        (1 - _alpha) * (_alpha - _porosity[_qp]) * _one_over_K + _porosity[_qp] * _one_over_Kf;
    _done_over_biot_modulus_dP[_qp] =
        -(1 - _alpha) * _dporosity_dP[_qp] * _one_over_K + _dporosity_dP[_qp] * _one_over_Kf;
    _done_over_biot_modulus_dep[_qp] =
        -(1 - _alpha) * _dporosity_dep[_qp] * _one_over_K + _dporosity_dep[_qp] * _one_over_Kf;
  }
}
