//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseObject.h"
#include "MooseApp.h"
#include "MooseUtils.h"
#include "Factory.h"

class FEProblem;
class FEProblemBase;
class EigenProblem;
class SubProblem;
class SystemBase;
class AuxiliarySystem;
class TransientBase;

InputParameters
MooseObject::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("enable", true, "Set the enabled status of the MooseObject.");
  params.addParam<std::vector<std::string>>(
      "control_tags",
      "Adds user-defined labels for accessing object parameters via control logic.");
  params.addParamNamesToGroup("enable control_tags", "Advanced");
  params.addPrivateParam<std::string>("_type");        // The name of the class being built
  params.addPrivateParam<std::string>("_object_name"); // The name passed to Factory::create
  params.addPrivateParam<std::string>("_unique_name"); // The unique name generated in the warehouse
  params.addPrivateParam<FEProblem *>("_fe_problem", nullptr);
  params.addPrivateParam<FEProblemBase *>("_fe_problem_base", nullptr);
  params.addPrivateParam<EigenProblem *>("_eigen_problem", nullptr);
  params.addPrivateParam<SubProblem *>("_subproblem", nullptr);
  params.addPrivateParam<SystemBase *>("_sys", nullptr);
  params.addPrivateParam<SystemBase *>("_nl_sys", nullptr);
  params.addPrivateParam<TransientBase *>("_executioner", nullptr);
  params.addPrivateParam<THREAD_ID>("_tid");
  params.addPrivateParam<bool>("_residual_object", false);
  return params;
}

MooseObject::MooseObject(const InputParameters & parameters)
  : ParallelParamObject(parameters.get<std::string>("_type"),
                        parameters.get<std::string>("_object_name"),
                        *parameters.getCheckedPointerParam<MooseApp *>("_moose_app"),
                        parameters),
    _enabled(getParam<bool>("enable"))
{
  if (Registry::isRegisteredObj(type()) && _app.getFactory().currentlyConstructing() != &parameters)
    mooseError(
        "This registered object was not constructed using the Factory, which is not supported.");
}

namespace
{
const std::string not_shared_error =
    "MooseObject::getSharedPtr() must only be called for objects that are managed by a "
    "shared pointer. Make sure this object is build using Factory::create(...).";
}

std::shared_ptr<MooseObject>
MooseObject::getSharedPtr()
{
  try
  {
    return shared_from_this();
  }
  catch (std::bad_weak_ptr &)
  {
    mooseError(not_shared_error);
  }
}

std::shared_ptr<const MooseObject>
MooseObject::getSharedPtr() const
{
  try
  {
    return shared_from_this();
  }
  catch (std::bad_weak_ptr &)
  {
    mooseError(not_shared_error);
  }
}
