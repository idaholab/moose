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

InputParameters
TestDeclareReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  return params;
}

TestDeclareReporter::TestDeclareReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(declareValue<int>("int", 1980)),
    _real(declareValue<Real>("real")),
    _vector(declareValue<std::vector<Real>>("vector")),
    _string(declareValue<std::string>("string")),
    _bcast_value(declareValue<Real, ReporterBroadcastContext>("broadcast")),
    _scatter_value(
        declareValue<dof_id_type, ReporterScatterContext>("scatter", _values_to_scatter)),
    _gather_value(
        declareValue<std::vector<dof_id_type>, ReporterGatherContext>("gather", _values_to_gather))
{
  if (processor_id() == 0)
    for (dof_id_type rank = 0; rank < n_processors(); ++rank)
      _values_to_scatter.push_back(rank);

  _values_to_gather.resize(1, processor_id());
}

void
TestDeclareReporter::execute()
{
  _int += 1;
  _real = 1.2345;
  _vector = {1, 1.1, 1.2};
  _string = "string";

  if (processor_id() == 0)
    _bcast_value = 42;
}

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
    _int(getReporterValue<int>("int_reporter")),
    _int_old(getReporterValue<int>("int_reporter", 1)),
    _real(getReporterValue<Real>("real_reporter")),
    _vector(getReporterValue<std::vector<Real>>("vector_reporter")),
    _string(getReporterValue<std::string>("string_reporter")),
    _bcast_value(getReporterValue<Real>("broadcast_reporter")),
    _scatter_value(getReporterValue<dof_id_type>("scatter_reporter", REPORTER_MODE_DISTRIBUTED)),
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
