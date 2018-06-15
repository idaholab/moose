//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTimeStepPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

#include "libmesh/quadrature.h"

#include <algorithm>
#include <limits>

registerMooseObject("TensorMechanicsApp", MaterialTimeStepPostprocessor);

template <>
InputParameters
validParams<MaterialTimeStepPostprocessor>()
{
  InputParameters params = validParams<ElementPostprocessor>();

  params.addClassDescription("This postprocessor estimates a timestep that reduces the increment "
                             "change in a material property below a given threshold.");

  params.addParam<bool>("use_matl_timestep_limit",
                        true,
                        "if true, the time step is limited by the minimum value of the "
                        "matl_timestep_limit property");

  params.addParam<MaterialPropertyName>("elements_changed_property",
                                        "name of the property used to evaluate if an"
                                        "element have changed");

  params.addRangeCheckedParam<int>("elements_changed_number",
                                   "elements_changed_number > 0",
                                   "number of elements used to determine if a change of"
                                   "time step is required");

  params.addRangeCheckedParam<Real>(
      "tolerance", "tolerance > 0", "tolerance used to determine if an element has changed");

  return params;
}

MaterialTimeStepPostprocessor::MaterialTimeStepPostprocessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _use_matl_time_step(getParam<bool>("use_matl_timestep_limit")),
    _matl_time_step(_use_matl_time_step ? &getMaterialPropertyByName<Real>("matl_timestep_limit")
                                        : nullptr),
    _matl_value(std::numeric_limits<Real>::max()),
    _use_elements_changed(parameters.isParamSetByUser("elements_changed_property")),
    _changed_property(_use_elements_changed
                          ? &getMaterialPropertyByName<Real>(
                                getParam<MaterialPropertyName>("elements_changed_property"))
                          : nullptr),
    _changed_property_old(_use_elements_changed
                              ? &getMaterialPropertyOldByName<Real>(
                                    getParam<MaterialPropertyName>("elements_changed_property"))
                              : nullptr),
    _target(isParamValid("elements_changed_number") ? getParam<int>("elements_changed_number") : 0),
    _count(0),
    _tolerance(parameters.isParamSetByUser("tolerance") ? getParam<Real>("tolerance")
                                                        : TOLERANCE * TOLERANCE),
    _qp(0)
{
  if (_use_elements_changed && !parameters.isParamSetByUser("elements_changed_number"))
    paramError("elements_changed_number",
               "needs to be set when elements_changed_property is defined");

  if (!_use_matl_time_step && !_use_elements_changed)
    mooseError("either use_matl_time_step needs to be true or elements_changed_property defined");
}

void
MaterialTimeStepPostprocessor::initialize()
{
  _matl_value = std::numeric_limits<Real>::max(); // start w/ the min
  _count = 0;
}

void
MaterialTimeStepPostprocessor::execute()
{
  if (_use_matl_time_step)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _matl_value = std::min(_matl_value, (*_matl_time_step)[_qp]);
  }

  if (_use_elements_changed)
  {
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (!MooseUtils::absoluteFuzzyEqual(
              (*_changed_property)[_qp], (*_changed_property_old)[_qp], _tolerance))
      {
        ++_count;
        return;
      }
    }
  }
}

Real
MaterialTimeStepPostprocessor::getValue()
{
  gatherMin(_matl_value);
  gatherSum(_count);

  if (_count == 0 || !_use_elements_changed)
    return _matl_value;

  return std::min(_dt * (Real)_target / (Real)_count, _matl_value);
}

void
MaterialTimeStepPostprocessor::threadJoin(const UserObject & y)
{
  const MaterialTimeStepPostprocessor & pps = static_cast<const MaterialTimeStepPostprocessor &>(y);
  if (_use_matl_time_step)
    _matl_value = std::min(_matl_value, pps._matl_value);
  if (_use_elements_changed)
    _count += pps._count;
}
