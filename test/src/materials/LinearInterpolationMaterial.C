/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "LinearInterpolationMaterial.h"

template <>
InputParameters
validParams<LinearInterpolationMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>(
      "prop_name", "The property name that will contain the piecewise function");
  params.addRequiredParam<std::vector<Real>>(
      "independent_vals", "The vector of indepedent values for building the piecewise function");
  params.addRequiredParam<std::vector<Real>>(
      "dependent_vals", "The vector of depedent values for building the piecewise function");
  params.addParam<bool>("use_poly_fit",
                        false,
                        "Setting to true will use polynomial fit instead of linear interpolation");
  return params;
}

LinearInterpolationMaterial::~LinearInterpolationMaterial()
{
  delete _poly_fit;
  delete _linear_interp;
}

LinearInterpolationMaterial::LinearInterpolationMaterial(const InputParameters & parameters)
  : Material(parameters),
    _use_poly_fit(getParam<bool>("use_poly_fit")),
    _linear_interp(NULL),
    _poly_fit(NULL),
    _property(declareProperty<Real>(getParam<std::string>("prop_name")))
{
  if (_use_poly_fit)
  {
    _poly_fit = new PolynomialFit(getParam<std::vector<Real>>("independent_vals"),
                                  getParam<std::vector<Real>>("dependent_vals"),
                                  4);

    _poly_fit->generate();
    _poly_fit->dumpSampleFile(
        getParam<std::string>("prop_name"), "X position", getParam<std::string>("prop_name"));
  }
  else
  {
    try
    {

      _linear_interp = new LinearInterpolation(getParam<std::vector<Real>>("independent_vals"),
                                               getParam<std::vector<Real>>("dependent_vals"));
    }
    catch (std::domain_error & e)
    {
      mooseError("In LinearInterpolationMaterial ", _name, ": ", e.what());
    }

    _linear_interp->dumpSampleFile(
        getParam<std::string>("prop_name"), "X position", getParam<std::string>("prop_name"));
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
