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

#include "PiecewiseLinearInterpolationMaterial.h"

template <>
InputParameters
validParams<PiecewiseLinearInterpolationMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute a property using a piecewise linear interpolation to define "
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
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  return params;
}

PiecewiseLinearInterpolationMaterial::PiecewiseLinearInterpolationMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_name(getParam<std::string>("property")),
    _coupled_var(coupledValue("variable")),
    _scale_factor(getParam<Real>("scale_factor"))
{
  std::vector<Real> x;
  std::vector<Real> y;

  if ((parameters.isParamValid("x")) || (parameters.isParamValid("y")))
  {
    if (!((parameters.isParamValid("x")) && (parameters.isParamValid("y"))))
      mooseError("In PiecewiseLinearInterpolationMaterial ",
                 _name,
                 ": Both 'x' and 'y' must be specified if either one is specified.");

    if (parameters.isParamValid("xy_data"))
      mooseError("In PiecewiseLinearInterpolationMaterial ",
                 _name,
                 ": Cannot specify 'x', 'y', and 'xy_data' together.");

    x = getParam<std::vector<Real>>("x");
    y = getParam<std::vector<Real>>("y");
  }
  else if (parameters.isParamValid("xy_data"))
  {
    std::vector<Real> xy = getParam<std::vector<Real>>("xy_data");
    unsigned int xy_size = xy.size();
    if (xy_size % 2 != 0)
      mooseError("In PiecewiseLinearInterpolationMaterial ",
                 _name,
                 ": Length of data provided in 'xy_data' must be a multiple of 2.");

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
    mooseError("In PiecewiseLinearInterpolationMaterial ", _name, ": ", e.what());
  }

  _property = &declareProperty<Real>(_prop_name);
  const VariableName & vname = getVar("variable", 0)->name();
  _dproperty = &declarePropertyDerivative<Real>(_prop_name, vname);
}

void
PiecewiseLinearInterpolationMaterial::computeQpProperties()
{
  (*_property)[_qp] = _scale_factor * _linear_interp->sample(_coupled_var[_qp]);
  (*_dproperty)[_qp] = _scale_factor * _linear_interp->sampleDerivative(_coupled_var[_qp]);
}
