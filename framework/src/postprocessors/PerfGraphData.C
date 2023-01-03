//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphData.h"

registerMooseObject("MooseApp", PerfGraphData);

InputParameters
PerfGraphData::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Retrieves performance information about a section from the PerfGraph.");

  params.addRequiredParam<std::string>("section_name", "The name of the section to get data for");
  params.addRequiredParam<MooseEnum>(
      "data_type", PerfGraph::dataTypeEnum(), "The type of data to retrieve for the section_name");
  params.addParam<bool>("must_exist",
                        true,
                        "Whether or not the section must exist; if false and the section does not "
                        "exist, the value is set to zero");

  return params;
}

PerfGraphData::PerfGraphData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _data_type(getParam<MooseEnum>("data_type")),
    _section_name(getParam<std::string>("section_name")),
    _must_exist(getParam<bool>("must_exist"))
{
}

Real
PerfGraphData::getValue()
{
  if (!_fe_problem.checkingUOAuxState())
    _current_data = perfGraph().sectionData(
        static_cast<PerfGraph::DataType>(_data_type), _section_name, _must_exist);

  return _current_data;
}
