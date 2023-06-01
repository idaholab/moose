//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeastSquaresFit.h"
#include "VectorPostprocessorInterface.h"
#include "PolynomialFit.h"

registerMooseObject("MooseApp", LeastSquaresFit);

InputParameters
LeastSquaresFit::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor",
      "The vectorpostprocessor on whose values we perform a least squares fit");
  params.addRequiredParam<std::string>("x_name", "The name of the independent variable");
  params.addRequiredParam<std::string>("y_name", "The name of the dependent variable");
  params.addRequiredParam<unsigned int>("order", "The order of the polynomial fit");
  params.addParam<bool>(
      "truncate_order",
      true,
      "Truncate the order of the fitted polynomial if an insufficient number of data points are "
      "provided. If this is set to false, an error will be generated in that case.");
  params.addParam<unsigned int>("num_samples", "The number of samples to be output");
  params.addParam<Real>(
      "x_scale", 1.0, "Value used to scale x values (scaling is done after shifting)");
  params.addParam<Real>(
      "x_shift", 0.0, "Value used to shift x values (shifting is done before scaling)");
  params.addParam<Real>(
      "y_scale", 1.0, "Value used to scale y values (scaling is done after shifting)");
  params.addParam<Real>(
      "y_shift", 0.0, "Value used to shift y values (shifting is done before scaling)");
  params.addParam<Real>("sample_x_min", "The minimum x value of the of samples to be output");
  params.addParam<Real>("sample_x_max", "The maximum x value of the of samples to be output");
  MooseEnum output_type("Coefficients Samples", "Coefficients");
  params.addParam<MooseEnum>(
      "output", output_type, "The quantity to output.  Options are: " + output_type.getRawNames());
  params.addClassDescription("Performs a polynomial least squares fit on the data contained in "
                             "another VectorPostprocessor");

  return params;
}

LeastSquaresFit::LeastSquaresFit(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vectorpostprocessor")),
    _order(parameters.get<unsigned int>("order")),
    _truncate_order(parameters.get<bool>("truncate_order")),
    _x_name(getParam<std::string>("x_name")),
    _y_name(getParam<std::string>("y_name")),
    _x_values(getVectorPostprocessorValue("vectorpostprocessor", _x_name)),
    _y_values(getVectorPostprocessorValue("vectorpostprocessor", _y_name)),
    _output_type(getParam<MooseEnum>("output")),
    _num_samples(0),
    _x_scale(parameters.get<Real>("x_scale")),
    _x_shift(parameters.get<Real>("x_shift")),
    _y_scale(parameters.get<Real>("y_scale")),
    _y_shift(parameters.get<Real>("y_shift")),
    _have_sample_x_min(isParamValid("sample_x_min")),
    _have_sample_x_max(isParamValid("sample_x_max")),
    _sample_x(NULL),
    _sample_y(NULL),
    _coeffs(NULL)
{
  if (_output_type == "Samples")
  {
    if (isParamValid("num_samples"))
      _num_samples = getParam<unsigned int>("num_samples");
    else
      mooseError("In LeastSquaresFit num_samples parameter must be provided with output=Samples");

    if (_have_sample_x_min)
      _sample_x_min = getParam<Real>("sample_x_min");
    if (_have_sample_x_max)
      _sample_x_max = getParam<Real>("sample_x_max");

    _sample_x = &declareVector(_x_name);
    _sample_y = &declareVector(_y_name);
  }
  else
  {
    if (isParamValid("num_samples"))
      mooseWarning("In LeastSquaresFit num_samples parameter is unused with output=Coefficients");
    _coeffs = &declareVector("coefficients");
  }

  if (_output_type == "Samples")
  {
    _sample_x->resize(_num_samples);
    _sample_y->resize(_num_samples);
  }
  else
    _coeffs->resize(_order + 1);
}

void
LeastSquaresFit::initialize()
{
  if (_output_type == "Samples")
  {
    _sample_x->clear();
    _sample_y->clear();
  }
  else
    _coeffs->clear();
}

void
LeastSquaresFit::execute()
{
  if (_x_values.size() != _y_values.size())
    mooseError("Size of data in x_values and y_values must be equal");
  if (_x_values.size() == 0)
    mooseError("Size of data in x_values and y_values must be > 0");

  // Create a copy of _x_values that we can modify.
  std::vector<Real> x_values(_x_values.begin(), _x_values.end());
  std::vector<Real> y_values(_y_values.begin(), _y_values.end());

  for (MooseIndex(_x_values) i = 0; i < _x_values.size(); ++i)
  {
    x_values[i] = (x_values[i] + _x_shift) * _x_scale;
    y_values[i] = (y_values[i] + _y_shift) * _y_scale;
  }

  PolynomialFit pf(x_values, y_values, _order, _truncate_order);
  pf.generate();

  if (_output_type == "Samples")
  {
    Real x_min;
    if (_have_sample_x_min)
      x_min = _sample_x_min;
    else
      x_min = *(std::min_element(x_values.begin(), x_values.end()));

    Real x_max;
    if (_have_sample_x_max)
      x_max = _sample_x_max;
    else
      x_max = *(std::max_element(x_values.begin(), x_values.end()));

    Real x_span = x_max - x_min;

    for (unsigned int i = 0; i < _num_samples; ++i)
    {
      Real x = x_min + static_cast<Real>(i) / _num_samples * x_span;
      _sample_x->push_back(x);
      _sample_y->push_back(pf.sample(x));
    }
  }
  else
    *_coeffs = pf.getCoefficients();
}
