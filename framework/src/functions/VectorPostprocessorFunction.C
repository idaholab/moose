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
  params.addParam<bool>(
      "parallel_sync",
      true,
      "Whether or not this Function should be synced to all processors when running in parallel.");

  MooseEnum component("x=0 y=1 z=2 time=3", "time");
  params.addParam<MooseEnum>(
      "component",
      component,
      "Component of the function evaluation point used to sample the VectorPostprocessor");

  params.addClassDescription(
      "Provides piecewise linear interpolation of from two columns of a VectorPostprocessor");

  return params;
}

VectorPostprocessorFunction::VectorPostprocessorFunction(const InputParameters & parameters)
  : Function(parameters),
    VectorPostprocessorInterface(this),
    _argument_column(getVectorPostprocessorValue("vectorpostprocessor_name",
                                                 getParam<std::string>("argument_column"),
                                                 getParam<bool>("parallel_sync"))),
    _value_column(getVectorPostprocessorValue("vectorpostprocessor_name",
                                              getParam<std::string>("value_column"),
                                              getParam<bool>("parallel_sync"))),
    _component(getParam<MooseEnum>("component")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _last_update(
        {std::numeric_limits<Real>::lowest(), libMesh::invalid_uint, libMesh::invalid_uint})
{
  try
  {
    _linear_interp = std::make_unique<LinearInterpolation>(_argument_column, _value_column);
  }
  catch (std::domain_error & e)
  {
    mooseError("In VectorPostprocessorFunction ", _name, ": ", e.what());
  }
}

Real
VectorPostprocessorFunction::value(Real t, const Point & p) const
{
  return valueInternal(t, p);
}

ADReal
VectorPostprocessorFunction::value(const ADReal & t, const ADPoint & p) const
{
  return valueInternal(t, p);
}

template <typename T, typename P>
T
VectorPostprocessorFunction::valueInternal(const T & t, const P & p) const
{
  if (_argument_column.empty())
    return 0.0;

  const std::tuple<Real, unsigned int, unsigned int> now = {MetaPhysicL::raw_value(t),
                                                            _fe_problem.nNonlinearIterations(),
                                                            _fe_problem.nLinearIterations()};

  if (now != _last_update)
  {
    _linear_interp->setData(_argument_column, _value_column);
    _last_update = now;
  }

  const T x = _component == 3 ? t : p(_component);
  return _linear_interp->sample(x);
}
