//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSetupCount.h"

#include "FVSetupCounter.h"
#include "FVInitialConditionBase.h"
#include "FVInitialConditionWarehouse.h"
#include "Attributes.h"

registerMooseObject("MooseTestApp", FVSetupCount);

InputParameters
FVSetupCount::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::string>("object",
                                       "The name of the finite volume object to report on");
  params.addRequiredParam<MooseEnum>("count_type",
                                     MooseEnum("INITIAL TIMESTEP CUSTOM"),
                                     "The setup method to report the call count of");
  params.addParam<std::string>("exec_flag",
                               "With count_type = CUSTOM, count only the calls made with this "
                               "execution flag. Many flags route through customSetup, so an "
                               "aggregate count is dominated by ones unrelated to finite volume.");
  params.addClassDescription("Reports how many times a named finite volume object has received a "
                             "given setup method, for testing setup dispatch.");
  return params;
}

FVSetupCount::FVSetupCount(const InputParameters & params)
  : GeneralPostprocessor(params),
    _object_name(getParam<std::string>("object")),
    _count_type(getParam<MooseEnum>("count_type")),
    _exec_flag(isParamValid("exec_flag") ? getParam<std::string>("exec_flag") : "")
{
  if (!_exec_flag.empty() && _count_type != "CUSTOM")
    paramError("exec_flag", "Only meaningful with 'count_type = CUSTOM'");
}

const FVSetupCounter &
FVSetupCount::getCounter() const
{
  std::vector<MooseObject *> named_objects;
  _fe_problem.theWarehouse().query().condition<AttribName>(_object_name).queryInto(named_objects);

  std::vector<const FVSetupCounter *> fv_objects;
  for (auto * object : named_objects)
    if (const auto * counter = dynamic_cast<const FVSetupCounter *>(object))
      fv_objects.push_back(counter);

  if (fv_objects.size() == 1)
    return *fv_objects[0];
  if (fv_objects.size() > 1)
    mooseError("Found ", fv_objects.size(), " finite volume objects named '", _object_name, "'");

  // Finite volume initial conditions are not added to TheWarehouse, so look for one of those
  for (const auto & ic : _fe_problem.getFVInitialConditionWarehouse().getActiveObjects())
    if (ic->name() == _object_name)
    {
      const auto * counter = dynamic_cast<const FVSetupCounter *>(ic.get());
      if (!counter)
        mooseError("Initial condition '", _object_name, "' does not count its setup calls");
      return *counter;
    }

  mooseError("No finite volume object named '", _object_name, "' was found");
}

PostprocessorValue
FVSetupCount::getValue() const
{
  const std::string key = _exec_flag.empty() ? std::string(_count_type) : "CUSTOM_" + _exec_flag;
  return getCounter().getSetupCount(key);
}
