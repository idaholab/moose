#include "WeightedAverageMaterial.h"

registerMooseObject("RELAP7App", WeightedAverageMaterial);

template <>
InputParameters
validParams<WeightedAverageMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addClassDescription(
      "Weighted average of material properties using aux variables as weights");

  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "The name of the material property where the average is stored");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("values",
                                                             "Vector of values to average");
  params.addRequiredCoupledVar("weights", "Vector of weights for each value");

  return params;
}

WeightedAverageMaterial::WeightedAverageMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop(declareProperty<Real>(getParam<MaterialPropertyName>("prop_name"))),
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
    _values.push_back(&getMaterialPropertyByName<Real>(prop_names[i]));
    _weights.push_back(&coupledValue("weights", i));
  }
}

void
WeightedAverageMaterial::computeQpProperties()
{
  Real weight_total = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    weight_total += (*(_weights[i]))[_qp];

  Real weighted_sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    weighted_sum += (*(_weights[i]))[_qp] * (*(_values[i]))[_qp];

  _prop[_qp] = weighted_sum / weight_total;
}
