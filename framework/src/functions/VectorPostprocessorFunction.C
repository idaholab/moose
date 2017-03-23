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

#include "VectorPostprocessorFunction.h"

template <>
InputParameters
validParams<VectorPostprocessorFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 3",
      "Component of the function evaluation point used to sample the VectorPostprocessor");
  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor_name", "The name of the PointValueSampler that you want to use");
  params.addRequiredParam<std::string>(
      "argument_column",
      "VectorPostprocessor column tabulating the abscissa of the sampled function");
  params.addRequiredParam<std::string>("value_column",
                                       "VectorPostprocessor column tabulating the "
                                       "ordinate (function values) of the sampled "
                                       "function");
  return params;
}

VectorPostprocessorFunction::VectorPostprocessorFunction(const InputParameters & parameters)
  : Function(parameters),
    VectorPostprocessorInterface(this),
    _component(parameters.get<unsigned int>("component")),
    _argument_column(getVectorPostprocessorValue("vectorpostprocessor_name",
                                                 getParam<std::string>("argument_column"))),
    _value_column(getVectorPostprocessorValue("vectorpostprocessor_name",
                                              getParam<std::string>("value_column")))
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
VectorPostprocessorFunction::value(Real /*t*/, const Point & p)
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
  return _linear_interp->sample(p(_component));
}
