//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWeightedAverageMaterial.h"

registerMooseObject("ThermalHydraulicsApp", ADWeightedAverageMaterial);

InputParameters
ADWeightedAverageMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription(
      "Weighted average of material properties using aux variables as weights");

  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "The name of the material property where the average is stored");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("values",
                                                             "Vector of values to average");
  params.addRequiredCoupledVar("weights", "Vector of weights for each value");

  return params;
}

ADWeightedAverageMaterial::ADWeightedAverageMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop(declareADProperty<Real>(getParam<MaterialPropertyName>("prop_name"))),
    _n_values(getParam<std::vector<MaterialPropertyName>>("values").size())
{
  // make sure that number of weights equals the number of values
  if (coupledComponents("weights") != _n_values)
    mooseError(name(), ": The number of weights must equal the number of values");

  // get all of the variable values
  const std::vector<MaterialPropertyName> & prop_names =
      getParam<std::vector<MaterialPropertyName>>("values");
  for (unsigned int i = 0; i < _n_values; i++)
  {
    _values.push_back(&getADMaterialPropertyByName<Real>(prop_names[i]));
    _weights.push_back(&adCoupledValue("weights", i));
  }
}

void
ADWeightedAverageMaterial::computeQpProperties()
{
  ADReal weight_total = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    weight_total += (*(_weights[i]))[_qp];

  ADReal weighted_sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    weighted_sum += (*(_weights[i]))[_qp] * (*(_values[i]))[_qp];

  _prop[_qp] = weighted_sum / weight_total;
}
