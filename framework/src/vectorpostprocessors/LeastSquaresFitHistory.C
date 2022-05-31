//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeastSquaresFitHistory.h"
#include "VectorPostprocessorInterface.h"
#include "PolynomialFit.h"
#include "Conversion.h"

registerMooseObject("MooseApp", LeastSquaresFitHistory);

InputParameters
LeastSquaresFitHistory::validParams()
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
  params.addParam<Real>(
      "x_scale", 1.0, "Value used to scale x values (scaling is done after shifting)");
  params.addParam<Real>(
      "x_shift", 0.0, "Value used to shift x values (shifting is done before scaling)");
  params.addParam<Real>(
      "y_scale", 1.0, "Value used to scale y values (scaling is done after shifting)");
  params.addParam<Real>(
      "y_shift", 0.0, "Value used to shift y values (shifting is done before scaling)");
  params.addClassDescription(
      "Performs a polynomial least squares fit on the data contained in "
      "another VectorPostprocessor and stores the full time history of the coefficients");

  params.set<bool>("contains_complete_history") = true;
  params.addParamNamesToGroup("contains_complete_history", "Advanced");

  return params;
}

LeastSquaresFitHistory::LeastSquaresFitHistory(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vectorpostprocessor")),
    _order(parameters.get<unsigned int>("order")),
    _truncate_order(parameters.get<bool>("truncate_order")),
    _x_name(getParam<std::string>("x_name")),
    _y_name(getParam<std::string>("y_name")),
    _x_values(getVectorPostprocessorValue("vectorpostprocessor", _x_name)),
    _y_values(getVectorPostprocessorValue("vectorpostprocessor", _y_name)),
    _x_scale(parameters.get<Real>("x_scale")),
    _x_shift(parameters.get<Real>("x_shift")),
    _y_scale(parameters.get<Real>("y_scale")),
    _y_shift(parameters.get<Real>("y_shift"))
{
  _coeffs.resize(_order + 1);
  for (unsigned int i = 0; i < _coeffs.size(); ++i)
    _coeffs[i] = &declareVector("coef_" + Moose::stringify(i));
  _times = &declareVector("time");
}

void
LeastSquaresFitHistory::initialize()
{
  // no reset/clear needed since contains complete history
}

void
LeastSquaresFitHistory::execute()
{
  if (_x_values.size() != _y_values.size())
    mooseError("In LeastSquresFitTimeHistory size of data in x_values and y_values must be equal");
  if (_x_values.size() == 0)
    mooseError("In LeastSquresFitTimeHistory size of data in x_values and y_values must be > 0");

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

  std::vector<Real> coeffs = pf.getCoefficients();
  mooseAssert(coeffs.size() == _coeffs.size(),
              "Sizes of current coefficients and vector of coefficient vectors must match");
  for (MooseIndex(coeffs) i = 0; i < coeffs.size(); ++i)
    _coeffs[i]->push_back(coeffs[i]);

  _times->push_back(_t);
}
