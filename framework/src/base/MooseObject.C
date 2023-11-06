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

class FEProblem;
class FEProblemBase;
class EigenProblem;
class SubProblem;
class SystemBase;
class AuxiliarySystem;
class Transient;

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
  params.addPrivateParam<Transient *>("_executioner", nullptr);
  params.addPrivateParam<THREAD_ID>("_tid");
  params.addPrivateParam<bool>("_residual_object", false);
  return params;
}

MooseObject::MooseObject(const InputParameters & parameters)
  : MooseBase(parameters.get<std::string>("_type"),
              parameters.get<std::string>("_object_name"),
              *parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    MooseBaseParameterInterface(parameters, this),
    MooseBaseErrorInterface(this),
    ParallelObject(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    DataFileInterface<MooseObject>(*this),
    _enabled(getParam<bool>("enable"))
{
}
