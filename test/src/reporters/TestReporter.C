//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestReporter.h"

registerMooseObject("MooseTestApp", TestDeclareReporter);
registerMooseObject("MooseTestApp", TestGetReporter);
registerMooseObject("MooseTestApp", TestDeclareInitialSetupReporter);
registerMooseObject("MooseTestApp", TestGetReporterDeclaredInInitialSetupReporter);
registerMooseObject("MooseTestApp", TestDeclareErrorsReporter);

InputParameters
TestDeclareReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  // MooseDocs:param_begin
  params.addParam<ReporterValueName>("int_name", "int", "The name of the integer data");
  // MooseDocs:param_end
  params.addParam<ReporterValueName>("distributed_vector_name", "Distributed vector to produce.");
  return params;
}

TestDeclareReporter::TestDeclareReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(declareValue<int>("int_name", 1980)), // MooseDocs:producer
    _real(declareValueByName<Real>("real")),
    _vector(declareValueByName<std::vector<Real>>("vector")),
    _string(declareValueByName<std::string>("string")),
    _bcast_value(declareValueByName<Real, ReporterBroadcastContext>("broadcast")),
    _scatter_value(
        declareValueByName<dof_id_type, ReporterScatterContext>("scatter", _values_to_scatter)),
    _gather_value(declareValueByName<std::vector<dof_id_type>, ReporterGatherContext>(
        "gather")), // MooseDocs:gather
    _distributed_vector(isParamValid("distributed_vector_name")
                            ? &declareValue<std::vector<dof_id_type>>("distributed_vector_name",
                                                                      REPORTER_MODE_DISTRIBUTED)
                            : nullptr)
{
  if (processor_id() == 0)
    for (dof_id_type rank = 0; rank < n_processors(); ++rank)
      _values_to_scatter.push_back(rank);
}

// MooseDocs:execute_begin
void
TestDeclareReporter::execute()
{
  _int += 1;
  _real = 1.2345;
  _vector = {1, 1.1, 1.2};
  _string = "string";

  if (processor_id() == 0)
    _bcast_value = 42;

  _gather_value.resize(1, processor_id());

  if (_distributed_vector)
  {
    _distributed_vector->resize(_vector.size());
    (*_distributed_vector)[0] = processor_id();
    for (unsigned int i = 1; i < _distributed_vector->size(); ++i)
      (*_distributed_vector)[i] = (*_distributed_vector)[i - 1] * 10;
  }
}
// MooseDocs:execute_end

InputParameters
TestGetReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<ReporterName>("int_reporter", "'int' reporter name");
  params.addRequiredParam<ReporterName>("real_reporter", "'real' reporter name");
  params.addRequiredParam<ReporterName>("vector_reporter", "'vector' reporter name");
  params.addRequiredParam<ReporterName>("string_reporter", "'string' reporter name");
  params.addRequiredParam<ReporterName>("broadcast_reporter", "'broadcast' reporter name");
  params.addRequiredParam<ReporterName>("scatter_reporter", "'scatter' reporter name");
  params.addRequiredParam<ReporterName>("gather_reporter", "'gather' reporter name");
  return params;
}

TestGetReporter::TestGetReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(getReporterValue<int>("int_reporter")), // MooseDocs:consumer
    _int_old(getReporterValue<int>("int_reporter", 1)),
    _real(getReporterValue<Real>("real_reporter")),
    _vector(getReporterValue<std::vector<Real>>("vector_reporter")),
    _string(getReporterValue<std::string>("string_reporter")),
    _bcast_value(getReporterValue<Real>("broadcast_reporter")),
    _scatter_value(getReporterValue<dof_id_type>("scatter_reporter")),
    _gather_value(getReporterValue<std::vector<dof_id_type>>("gather_reporter"))
{
}

void
TestGetReporter::execute()
{
  if (_int != 1980 + _t_step)
    mooseError("int reporter test failed: ", _int, " != ", 1980 + _t_step);
  if (_real != 1.2345)
    mooseError("Real reporter test failed");
  if (_vector != std::vector<Real>({1., 1.1, 1.2}))
    mooseError("std::vector<Real> reporter test failed");
  if (_string != "string")
    mooseError("std::string reporter test failed");

  if (_t_step == 0 && _int_old != 1980)
    mooseError("int_old on timestep 0 failed: ", _int_old, " != ", 1980);
  if (_t_step > 0 && _int_old != 1980 + (_t_step - 1))
    mooseError(
        "int_old on timestep ", _t_step, " failed: ", _int_old, " != ", 1980 + (_t_step - 1));

  if (_bcast_value != 42)
    mooseError("Broadcast reporter test failed");

  if (_scatter_value != processor_id())
    mooseError("Scatter reporter test failed");

  if (processor_id() == 0)
  {
    std::vector<dof_id_type> gold;
    for (dof_id_type id = 0; id < n_processors(); ++id)
      gold.push_back(id);
    if (_gather_value != gold)
      mooseError("Gather reporter test failed!");
  }
}

InputParameters
TestDeclareInitialSetupReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<Real>("value", "The value to report.");
  return params;
}

TestDeclareInitialSetupReporter::TestDeclareInitialSetupReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{
}

void
TestDeclareInitialSetupReporter::initialSetup()
{
  Real & value = declareValueByName<Real>("value");
  value = getParam<Real>("value");
}

InputParameters
TestGetReporterDeclaredInInitialSetupReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<ReporterName>("other_reporter",
                                        "The reporter name that was declared in initialSetup");
  return params;
}

TestGetReporterDeclaredInInitialSetupReporter::TestGetReporterDeclaredInInitialSetupReporter(
    const InputParameters & parameters)
  : GeneralReporter(parameters),
    _value_declared_in_initial_setup(getReporterValue<Real>("other_reporter")),
    _the_value_of_the_reporter(declareValueByName<Real>("other_value"))
{
}

void
TestGetReporterDeclaredInInitialSetupReporter::execute()
{
  _the_value_of_the_reporter = _value_declared_in_initial_setup;
}

InputParameters
TestDeclareErrorsReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<ReporterValueName>("value", "A reporter value name");

  params.addParam<bool>("missing_param", false, "True to test the error for a missing parameter");
  params.addParam<bool>("bad_param", false, "True to test the error for a bad parameter type");
  params.addParam<bool>("already_declared", false, "Test declaring a value multiple times");
  params.addParam<bool>("requested_different_type",
                        false,
                        "Test declaring a value that has been requested with a differentt type");

  return params;
}

TestDeclareErrorsReporter::TestDeclareErrorsReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{
  if (getParam<bool>("missing_param"))
    declareValue<int>("some_missing_parm");
  if (getParam<bool>("bad_param"))
    declareValue<int>("bad_param");
  if (getParam<bool>("already_declared"))
  {
    declareValueByName<int>("value_name");
    declareValueByName<Real>("value_name");
  }
  if (getParam<bool>("requested_different_type"))
  {
    getReporterValueByName<int>(name() + "/value_name");
    declareValueByName<Real>("value_name");
  }
}
