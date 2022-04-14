//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearInterpolationMaterial.h"
#include "LinearInterpolation.h"
#include "PolynomialFit.h"

#include "libmesh/auto_ptr.h"

registerMooseObject("MooseTestApp", LinearInterpolationMaterial);

InputParameters
LinearInterpolationMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>(
      "prop_name", "The property name that will contain the piecewise function");
  params.addRequiredParam<std::vector<Real>>(
      "independent_vals", "The vector of independent values for building the piecewise function");
  params.addRequiredParam<std::vector<Real>>(
      "dependent_vals", "The vector of dependent values for building the piecewise function");
  params.addParam<bool>("use_poly_fit",
                        false,
                        "Setting to true will use polynomial fit instead of linear interpolation");
  return params;
}

LinearInterpolationMaterial::LinearInterpolationMaterial(const InputParameters & parameters)
  : Material(parameters),
    _use_poly_fit(getParam<bool>("use_poly_fit")),
    _linear_interp(nullptr),
    _poly_fit(nullptr),
    _property(declareProperty<Real>(getParam<std::string>("prop_name")))
{
  if (_use_poly_fit)
  {
    _poly_fit = std::make_unique<PolynomialFit>(getParam<std::vector<Real>>("independent_vals"),
                                                getParam<std::vector<Real>>("dependent_vals"),
                                                4);

    _poly_fit->generate();
  }
  else
  {
    try
    {

      _linear_interp =
          std::make_unique<LinearInterpolation>(getParam<std::vector<Real>>("independent_vals"),
                                                getParam<std::vector<Real>>("dependent_vals"));
    }
    catch (std::domain_error & e)
    {
      mooseError("In LinearInterpolationMaterial ", _name, ": ", e.what());
    }
  }
}

void
LinearInterpolationMaterial::computeQpProperties()
{
  // We are just going to sample with the current X coordinate

  if (_use_poly_fit)
    _property[_qp] = _poly_fit->sample(_q_point[_qp](0));
  else
    _property[_qp] = _linear_interp->sample(_q_point[_qp](0));
}
