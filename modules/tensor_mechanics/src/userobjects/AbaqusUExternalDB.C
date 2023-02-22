//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUExternalDB.h"
#include "AbaqusUtils.h"
#include "StepUserObject.h"

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("TensorMechanicsApp", AbaqusUExternalDB);

InputParameters
AbaqusUExternalDB::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params.addClassDescription("Coupling user object to use Abaqus UEXTERNALDB subroutines in MOOSE");
  params.addRequiredParam<FileName>(
      "plugin", "The path to the compiled dynamic library for the plugin you want to use");
  return params;
}

#ifndef METHOD
#error "The METHOD preprocessor symbol must be supplied by the build system."
#endif

AbaqusUExternalDB::AbaqusUExternalDB(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _uexternaldb(_library.getFunction<uexternaldb_t>("uexternaldb_")),
    _aqSTEP(0),
    _current_execute_on_flag(_fe_problem.getCurrentExecuteOnFlag())
{
  AbaqusUtils::setInputFile(_app.getInputFileName());
  AbaqusUtils::setCommunicator(&_communicator);
}

void
AbaqusUExternalDB::initialSetup()
{
  // Let's automatically detect uos and identify the one we are interested in.
  // If there is more than one, we assume something is off and error out.
  std::vector<const UserObject *> uos;
  _fe_problem.theWarehouse().query().condition<AttribSystem>("UserObject").queryInto(uos);

  std::vector<const StepUserObject *> step_uos;
  for (const auto & uo : uos)
  {
    const StepUserObject * possible_step_uo = dynamic_cast<const StepUserObject *>(uo);
    if (possible_step_uo)
      step_uos.push_back(possible_step_uo);
  }

  if (step_uos.size() > 1)
    mooseError("Your input file has multiple StepUserObjects. MOOSE currently only support one in "
               "AbaqusUExternalDB.");
  else if (step_uos.size() == 1)
    mooseInfo(
        "A StepUserObject, ",
        step_uos[0]->name(),
        ", has been identified and will be used to drive stepping behavior in AbaqusUExternalDB.");

  _step_user_object = step_uos.size() == 1 ? step_uos[0] : nullptr;
}

void
AbaqusUExternalDB::execute()
{
  // Obtain step number if user object was coupled (beginning of increment)
  if (_step_user_object)
    _aqSTEP = _step_user_object->getStep(_t - _dt);

  if (_current_execute_on_flag == EXEC_INITIAL)
    callPlugin(0);

  // TODO support 2 -> trigger saving for restart

  if (_current_execute_on_flag == EXEC_FINAL)
    callPlugin(3);

  if (_current_execute_on_flag == EXEC_TIMESTEP_BEGIN)
    callPlugin(1);

  if (_current_execute_on_flag == EXEC_TIMESTEP_END)
    callPlugin(2);
}

void
AbaqusUExternalDB::callPlugin(int lop)
{
  Real time[2] = {_t, _t - _dt};
  int lrestart = 0;

  // call plugin function
  _uexternaldb(&lop, &lrestart, time, &_dt, &_aqSTEP, &_t_step);
}
