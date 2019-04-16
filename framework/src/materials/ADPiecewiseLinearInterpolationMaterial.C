//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPiecewiseLinearInterpolationMaterial.h"

#include "MooseVariableFE.h"

registerADMooseObject("MooseApp", ADPiecewiseLinearInterpolationMaterial);

defineADValidParams(
    ADPiecewiseLinearInterpolationMaterial,
    ADMaterial,
    params.addClassDescription(
        "Compute a property using a piecewise linear interpolation to define "
        "its dependence on a variable");
    params.addRequiredParam<std::string>("property",
                                         "The name of the property this material will compute");
    params.addRequiredCoupledVar(
        "variable",
        "The name of the variable whose value is used as the abscissa in the interpolation");
    params.addParam<std::vector<Real>>("x", "The abscissa values");
    params.addParam<std::vector<Real>>("y", "The ordinate values");
    params.addParam<std::vector<Real>>("xy_data",
                                       "All function data, supplied in abscissa, ordinate pairs");
    params.addParam<Real>("scale_factor",
                          1.0,
                          "Scale factor to be applied to the ordinate values"););

template <ComputeStage compute_stage>
ADPiecewiseLinearInterpolationMaterial<compute_stage>::ADPiecewiseLinearInterpolationMaterial(
    const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _prop_name(adGetParam<std::string>("property")),
    _coupled_var(adCoupledValue("variable")),
    _scale_factor(adGetParam<Real>("scale_factor")),
    _property(adDeclareADProperty<Real>(_prop_name))
{
  std::vector<Real> x;
  std::vector<Real> y;

  if ((parameters.isParamValid("x")) || (parameters.isParamValid("y")))
  {
    if (!((parameters.isParamValid("x")) && (parameters.isParamValid("y"))))
      mooseError("In ", _name, ": Both 'x' and 'y' must be specified if either one is specified.");

    if (parameters.isParamValid("xy_data"))
      mooseError("In ", _name, ": Cannot specify 'x', 'y', and 'xy_data' together.");

    x = adGetParam<std::vector<Real>>("x");
    y = adGetParam<std::vector<Real>>("y");
  }
  else if (parameters.isParamValid("xy_data"))
  {
    std::vector<Real> xy = adGetParam<std::vector<Real>>("xy_data");
    unsigned int xy_size = xy.size();
    if (xy_size % 2 != 0)
      mooseError("In ", _name, ": Length of data provided in 'xy_data' must be a multiple of 2.");

    unsigned int x_size = xy_size / 2;
    x.reserve(x_size);
    y.reserve(x_size);
    for (unsigned int i = 0; i < xy_size / 2; ++i)
    {
      x.push_back(xy[i * 2]);
      y.push_back(xy[i * 2 + 1]);
    }
  }

  try
  {
    _linear_interp = libmesh_make_unique<LinearInterpolation>(x, y);
  }
  catch (std::domain_error & e)
  {
    mooseError("In ", _name, ": ", e.what());
  }
}

template <ComputeStage compute_stage>
void
ADPiecewiseLinearInterpolationMaterial<compute_stage>::computeQpProperties()
{
  _property[_qp].value() = _scale_factor * _linear_interp->sample(_coupled_var[_qp].value());
  _property[_qp].derivatives() = _scale_factor *
                                 _linear_interp->sampleDerivative(_coupled_var[_qp].value()) *
                                 _coupled_var[_qp].derivatives();
}
