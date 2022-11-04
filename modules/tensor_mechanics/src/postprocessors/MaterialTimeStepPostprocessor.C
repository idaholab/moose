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

InputParameters
MaterialTimeStepPostprocessor::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();

  params.addClassDescription("This postprocessor estimates a timestep that reduces the increment "
                             "change in a material property below a given threshold.");

  params.addParam<bool>("use_material_timestep_limit",
                        true,
                        "if true, the time step is limited by the minimum value of the "
                        "material_timestep_limit property");

  params.addParam<MaterialPropertyName>("elements_changed_property",
                                        "Name of the material property used to limit the time step "
                                        "if its value changes by more than "
                                        "'elements_changed_threshold' in at least "
                                        "'elements_changed' elements");

  params.addRangeCheckedParam<int>("elements_changed",
                                   "elements_changed > 0",
                                   "Maximum number of elements within which the property named in "
                                   "'elements_changed_property' is allowed to change by more than "
                                   "'elements_changed_threshold' before the time step is limited.");

  params.addRangeCheckedParam<Real>("elements_changed_threshold",
                                    "elements_changed_threshold' > 0",
                                    "Maximum permitted change in the value of "
                                    "'elements_changed_property' in 'elements_changed' elements "
                                    "before the time step is limited.");
  params.addRangeCheckedParam<Real>("maximum_value",
                                    std::numeric_limits<Real>::max(),
                                    "maximum_value>=0",
                                    "Maximum value returned by this postprocessor.");

  return params;
}

MaterialTimeStepPostprocessor::MaterialTimeStepPostprocessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _use_material_timestep_limit(getParam<bool>("use_material_timestep_limit")),
    _matl_time_step(_use_material_timestep_limit
                        ? &getMaterialPropertyByName<Real>("material_timestep_limit")
                        : nullptr),
    _matl_value(getParam<Real>("maximum_value")),
    _use_elements_changed(parameters.isParamSetByUser("elements_changed_property")),
    _changed_property(_use_elements_changed
                          ? &getMaterialPropertyByName<Real>(
                                getParam<MaterialPropertyName>("elements_changed_property"))
                          : nullptr),
    _changed_property_old(_use_elements_changed
                              ? &getMaterialPropertyOldByName<Real>(
                                    getParam<MaterialPropertyName>("elements_changed_property"))
                              : nullptr),
    _elements_changed(isParamValid("elements_changed") ? getParam<int>("elements_changed") : 0),
    _count(0),
    _elements_changed_threshold(parameters.isParamSetByUser("elements_changed_threshold'")
                                    ? getParam<Real>("elements_changed_threshold'")
                                    : TOLERANCE * TOLERANCE),
    _max(getParam<Real>("maximum_value")),
    _qp(0)
{
  if (_use_elements_changed && !parameters.isParamSetByUser("elements_changed"))
    paramError("elements_changed", "needs to be set when elements_changed_property is defined");

  if (!_use_material_timestep_limit && !_use_elements_changed)
    mooseError("either use_material_timestep_limit needs to be true or elements_changed_property "
               "defined");
}

void
MaterialTimeStepPostprocessor::initialize()
{
  _matl_value = _max; // start w/ the maximum allowed value
  _count = 0;
}

void
MaterialTimeStepPostprocessor::execute()
{
  if (_use_material_timestep_limit)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _matl_value = std::min(_matl_value, (*_matl_time_step)[_qp]);
  }

  if (_use_elements_changed)
  {
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (!MooseUtils::absoluteFuzzyEqual((*_changed_property)[_qp],
                                          (*_changed_property_old)[_qp],
                                          _elements_changed_threshold))
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
  if (_count == 0 || !_use_elements_changed)
    return _matl_value;

  return std::min(_dt * (Real)_elements_changed / (Real)_count, _matl_value);
}

void
MaterialTimeStepPostprocessor::finalize()
{
  gatherMin(_matl_value);
  gatherSum(_count);
}

void
MaterialTimeStepPostprocessor::threadJoin(const UserObject & y)
{
  const MaterialTimeStepPostprocessor & pps = static_cast<const MaterialTimeStepPostprocessor &>(y);
  if (_use_material_timestep_limit)
    _matl_value = std::min(_matl_value, pps._matl_value);
  if (_use_elements_changed)
    _count += pps._count;
}
