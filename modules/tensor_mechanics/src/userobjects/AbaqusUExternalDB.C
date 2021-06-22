//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUExternalDB.h"

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
    _library(std::string("-") + QUOTE(METHOD) + ".plugin"),
    _uexternaldb(_library.getFunction<uexternaldb_t>("uexternaldb_")),
    _current_execute_on_flag(_fe_problem.getCurrentExecuteOnFlag())
{
}

void
AbaqusUExternalDB::execute()
{
  if (_current_execute_on_flag == EXEC_FINAL)
    callPlugin(3);

  if (_current_execute_on_flag == EXEC_TIMESTEP_BEGIN)
    callPlugin(5);

  if (_current_execute_on_flag == EXEC_TIMESTEP_END)
    callPlugin(6);
}

void
AbaqusUExternalDB::callPlugin(int lop)
{
  Real time[2] = {_t, _t - _dt};
  int lrestart = 0;
  int kinc = 0; // ?

  // call plugin function
  _uexternaldb(&lop, &lrestart, time, &_dt, &_t_step, &kinc);
}
