//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElasticEnergyMaterialSAS.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PhaseFieldApp", ElasticEnergyMaterialSAS);

InputParameters
ElasticEnergyMaterialSAS::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBase::validParams();
  params.addClassDescription("Free energy material for the elastic energy contributions.");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<MaterialPropertyName>("fa_name", "Phase A material");
  params.addRequiredParam<MaterialPropertyName>("fb_name", "Phase B material");
  params.addDeprecatedCoupledVar("args",
                                 "Arguments of the free energy function",
                                 "args is deprecated, use 'coupled_variables' instead");
  params.addCoupledVar("coupled_variables",
                       "Vector of variable arguments of the free energy function");
  params.addCoupledVar("displacement_gradients",
                       "Vector of displacement gradient variables (see "
                       "Modules/PhaseField/DisplacementGradients "
                       "action)");
  params.addRequiredParam<std::string>("base_A", "Base name for the Phase A");
  params.addRequiredParam<std::string>("base_B", "Base name for the Phase B");
  params.addRequiredParam<MaterialPropertyName>(
      "h_name", "Name of the switching function material property for the given phase");
  return params;
}

ElasticEnergyMaterialSAS::ElasticEnergyMaterialSAS(const InputParameters & parameters)
  : DerivativeFunctionMaterialBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _prop_Fa(getMaterialProperty<Real>("fa_name")),
    _prop_Fb(getMaterialProperty<Real>("fb_name")),
    _h_name(getParam<MaterialPropertyName>("h_name")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _strain_A(getMaterialPropertyByName<RankTwoTensor>(_base_A + "elastic_strain")),
    _strain_B(getMaterialPropertyByName<RankTwoTensor>(_base_B + "elastic_strain")),
    _eigenstrain_B(getMaterialPropertyByName<RankTwoTensor>(_base_B + "eigenstrain")),
    _num_eta(coupledComponents("coupled_variables")),
    _eta(coupledValues("coupled_variables")),
    _eta_names(coupledNames("coupled_variables")),
    _h_eta(getMaterialProperty<Real>("h_name")),
    _dh_eta(_num_eta)
{
  for(unsigned int i = 0; i< _num_eta; ++i)
  {
    _dh_eta[i] = &getMaterialPropertyDerivativeByName<Real>(_h_name, _eta_names[i]);
  }
}

void
ElasticEnergyMaterialSAS::initialSetup()
{
  validateCoupling<RankTwoTensor>(_base_name + "elastic_strain");
}

Real
ElasticEnergyMaterialSAS::computeF()
{
  return (_h_eta[_qp] * _prop_Fb[_qp] + (1.0 - _h_eta[_qp]) * _prop_Fa[_qp]);
}

Real
ElasticEnergyMaterialSAS::computeDF(unsigned int i_var)
{
  unsigned int i = argIndex(i_var);

  return -1.0*(*_dh_eta[i])[_qp] * _stress[_qp].doubleContraction(_eigenstrain_B[_qp] + 0.5 * (_strain_B[_qp]-_strain_A[_qp]));
}
