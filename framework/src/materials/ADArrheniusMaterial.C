//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrheniusMaterial.h"
#include "Function.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", ADArrheniusMaterial);

InputParameters
ADArrheniusMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("temp", "Coupled Temperature");

  params.addRequiredParam<Real>("pre_exponential", "The pre-exponential term");

  params.addRequiredParam<Real>("activation_energy", "Activation energy for diffusion");
  params.addParam<Real>("ideal_gas_constant", 8.31446261815324, "Ideal gas constant");
  params.addParam<MaterialPropertyName>(
      "diffusivity_name", "D", "The name of the diffusivity variable");
  params.addClassDescription("Material model to compute diffusivity using an Arrhenius method");

  return params;
}

ADArrheniusMaterial::ADArrheniusMaterial(const InputParameters & parameters)
  : Material(parameters),

    _ad_temperature(adCoupledValue("temp")),
    _my_pre_exponential(isParamValid("pre_exponential") ? getParam<Real>("pre_exponential") : 0),
    _my_activation_energy(isParamValid("activation_energy") ? getParam<Real>("activation_energy")
                                                            : 0),
    _my_ideal_gas_constant(isParamValid("ideal_gas_constant") ? getParam<Real>("ideal_gas_constant")
                                                              : 8.31466261815324),
    _diffusivity_name(isParamValid("diffusivity_name")
                          ? getParam<MaterialPropertyName>("diffusivity_name")
                          : "diffusivity"),
    _pre_exponential(declareADProperty<Real>("pre_exponential")),
    _activation_energy(declareADProperty<Real>("activation_energy")),
    _ideal_gas_constant(declareADProperty<Real>("ideal_gas_constant")),
    _diffusivity(declareADProperty<Real>(_diffusivity_name))
{
}

void
ADArrheniusMaterial::setDerivatives(ADReal & prop, Real dprop_dT, const ADReal & ad_T)
{
  if (ad_T < 0)
    prop.derivatives() = 0;
  else
    prop.derivatives() = dprop_dT * ad_T.derivatives();
}

void
ADArrheniusMaterial::computeProperties()
{
  /// TODO: Set diffusivity here

  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    Real qp_temperature = 0;
    qp_temperature = MetaPhysicL::raw_value(_ad_temperature[qp]);
    if (qp_temperature < 0)
    {
      std::stringstream msg;
      msg << "WARNING:  In ADArrheniusMaterial:  negative temperature!\n"
          << "\tResetting to zero.\n"
          << "\t_qp: " << qp << "\n"
          << "\ttemp: " << qp_temperature << "\n"
          << "\telem: " << _current_elem->id() << "\n"
          << "\tproc: " << processor_id() << "\n";
      mooseWarning(msg.str());
      qp_temperature = 0;
    }
    _pre_exponential[qp] = _my_pre_exponential;
    _ideal_gas_constant[qp] = _my_ideal_gas_constant;
    _activation_energy[qp] = _my_activation_energy;
    _diffusivity[qp] = _pre_exponential[qp] * exp(_activation_energy[qp] /
                                                  (_ideal_gas_constant[qp] * _ad_temperature[qp]));
  }
}
