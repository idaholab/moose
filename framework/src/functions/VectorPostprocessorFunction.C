//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorFunction.h"

registerMooseObject("MooseApp", VectorPostprocessorFunction);

defineLegacyParams(VectorPostprocessorFunction);

InputParameters
VectorPostprocessorFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_name", "The name of the VectorPostprocessor that you want to use");
  params.addRequiredParam<std::string>(
      "argument_column",
      "VectorPostprocessor column tabulating the abscissa of the sampled function");
  params.addRequiredParam<std::string>("value_column",
                                       "VectorPostprocessor column tabulating the "
                                       "ordinate (function values) of the sampled "
                                       "function");
  params.addRangeCheckedParam<unsigned int>(
      "component",
      "component < 3",
      "Component of the function evaluation point used to sample the VectorPostprocessor");

  params.addClassDescription(
      "Provides piecewise linear interpolation of from two columns of a VectorPostprocessor");

  return params;
}

VectorPostprocessorFunction::VectorPostprocessorFunction(const InputParameters & parameters)
  : Function(parameters),
    VectorPostprocessorInterface(this),
    _argument_column(getVectorPostprocessorValue("vectorpostprocessor_name",
                                                 getParam<std::string>("argument_column"))),
    _value_column(getVectorPostprocessorValue("vectorpostprocessor_name",
                                              getParam<std::string>("value_column"))),
    _component(isParamValid("component") ? getParam<unsigned int>("component") : NO_COMPONENT)
{
  try
  {
    _linear_interp = libmesh_make_unique<LinearInterpolation>(_argument_column, _value_column);
  }
  catch (std::domain_error & e)
  {
    mooseError("In VectorPostprocessorFunction ", _name, ": ", e.what());
  }
}

Real
VectorPostprocessorFunction::value(Real t, const Point & p) const
{
  if (_argument_column.empty())
  {
    std::vector<Real> dummy{0};
    _linear_interp->setData(dummy, dummy);
  }
  else
  {
    // TODO: figure out a way to only reinitialize the interpolation only once per ...linear
    // iteration?
    _linear_interp->setData(_argument_column, _value_column);
  }
  const Real x = _component != NO_COMPONENT ? p(_component) : t;
  return _linear_interp->sample(x);
}

const unsigned int VectorPostprocessorFunction::NO_COMPONENT =
    std::numeric_limits<unsigned int>::max();
