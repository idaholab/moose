//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedKelvinVoigtModel.h"

registerMooseObject("TensorMechanicsApp", GeneralizedKelvinVoigtModel);

InputParameters
GeneralizedKelvinVoigtModel::validParams()
{
  InputParameters params = GeneralizedKelvinVoigtBase::validParams();
  params.addClassDescription(
      "Generalized Kelvin-Voigt model composed of a serial assembly of unit Kelvin-Voigt modules");
  params.addRequiredParam<Real>("young_modulus", "initial elastic modulus of the material");
  params.addRequiredParam<Real>("poisson_ratio", "initial poisson ratio of the material");
  params.addRequiredParam<std::vector<Real>>(
      "creep_modulus", "list of the elastic moduli of the different springs in the material");
  params.addRequiredParam<std::vector<Real>>(
      "creep_viscosity",
      "list of the characteristic times of the different dashpots in the material");
  params.addParam<std::vector<Real>>(
      "creep_ratio", "list of the poisson ratios of the different springs in the material");
  params.set<bool>("force_recompute_properties") = false;
  params.suppressParameter<bool>("force_recompute_properties");
  return params;
}

GeneralizedKelvinVoigtModel::GeneralizedKelvinVoigtModel(const InputParameters & parameters)
  : GeneralizedKelvinVoigtBase(parameters),
    _Ci(getParam<std::vector<Real>>("creep_modulus").size()),
    _eta_i(getParam<std::vector<Real>>("creep_viscosity")),
    _Si(getParam<std::vector<Real>>("creep_modulus").size())
{
  Real young_modulus = getParam<Real>("young_modulus");
  Real poisson_ratio = getParam<Real>("poisson_ratio");

  _C0.fillFromInputVector({young_modulus, poisson_ratio}, RankFourTensor::symmetric_isotropic_E_nu);
  _S0 = _C0.invSymm();

  std::vector<Real> creep_modulus = getParam<std::vector<Real>>("creep_modulus");
  std::vector<Real> creep_ratio;
  if (isParamValid("creep_ratio"))
    creep_ratio = getParam<std::vector<Real>>("creep_ratio");
  else
    creep_ratio.resize(_Ci.size(), poisson_ratio);

  if (creep_modulus.size() != _Ci.size())
    mooseError("incompatible number of creep moduli and viscosities");
  if (creep_ratio.size() != _Ci.size())
    mooseError("incompatible number of creep ratios and viscosities");
  if (!(_Ci.size() == _eta_i.size() || _Ci.size() + 1 == _eta_i.size()))
    mooseError("incompatible number of creep ratios and viscosities");

  for (unsigned int i = 0; i < _Ci.size(); ++i)
  {
    _Ci[i].fillFromInputVector({creep_modulus[i], creep_ratio[i]},
                               RankFourTensor::symmetric_isotropic_E_nu);
    _Si[i] = _Ci[i].invSymm();
  }

  for (unsigned int i = 0; i < _eta_i.size(); ++i)
  {
    if (_eta_i[i] < 0 || MooseUtils::absoluteFuzzyEqual(_eta_i[i], 0.0))
      mooseError("material viscosity must be strictly > 0");
  }

  _components = _eta_i.size();
  _has_longterm_dashpot = (_eta_i.size() == _Ci.size() + 1);

  issueGuarantee(_elasticity_tensor_name, Guarantee::ISOTROPIC);
  declareViscoelasticProperties();
}

void
GeneralizedKelvinVoigtModel::computeQpViscoelasticProperties()
{
  _first_elasticity_tensor[_qp] = _C0;

  for (unsigned int i = 0; i < _Ci.size(); ++i)
    (*_springs_elasticity_tensors[i])[_qp] = _Ci[i];

  for (unsigned int i = 0; i < _eta_i.size(); ++i)
    (*_dashpot_viscosities[i])[_qp] = _eta_i[i];
}

void
GeneralizedKelvinVoigtModel::computeQpViscoelasticPropertiesInv()
{
  (*_first_elasticity_tensor_inv)[_qp] = _S0;

  for (unsigned int i = 0; i < _Si.size(); ++i)
    (*_springs_elasticity_tensors_inv[i])[_qp] = _Si[i];
}
