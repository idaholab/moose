/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ThermalFractureIntegral.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<ThermalFractureIntegral>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Calculates summation of the derivative of the eigenstrains with respect to temparture.");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");
  return params;
}

ThermalFractureIntegral::ThermalFractureIntegral(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _deigenstrain_dT(_eigenstrain_names.size()),
    _total_deigenstrain_dT(declareProperty<RankTwoTensor>("total_deigenstrain_dT"))
{
  // Get the materials containing the derivatives of the eigenstrains wrt temperature
  VariableName temp_name = getVar("temperature", 0)->name();
  if (_eigenstrain_names.size() == 0)
    mooseWarning("No 'eigenstrain_names' specified for ThermalFractureIntegral when 'temperature' "
                 "is specified");

  for (unsigned int i = 0; i < _deigenstrain_dT.size(); ++i)
    _deigenstrain_dT[i] = &getMaterialPropertyDerivative<RankTwoTensor>(
        _base_name + _eigenstrain_names[i], temp_name);
}

void
ThermalFractureIntegral::computeQpProperties()
{
  _total_deigenstrain_dT[_qp] = ((*_deigenstrain_dT[0])[_qp]);
  for (unsigned int i = 1; i < _deigenstrain_dT.size(); ++i)
    _total_deigenstrain_dT[_qp] += (*_deigenstrain_dT[i])[_qp];
}
