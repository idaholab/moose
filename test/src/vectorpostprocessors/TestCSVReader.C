//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCSVReader.h"

registerMooseObject("MooseTestApp", TestCSVReader);

InputParameters
TestCSVReader::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addRequiredParam<VectorPostprocessorName>("vectorpostprocessor",
                                                   "The vector postprocessor to examine.");
  params.addRequiredParam<std::string>("vector",
                                       "The vector to consider from the VectorPostprocessor.");
  params.addRequiredParam<processor_id_type>("rank", "The CPU rank to compare.");
  params.addRequiredParam<std::vector<double>>("gold", "The data to compare against.");

  return params;
}

TestCSVReader::TestCSVReader(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _vpp_data(getVectorPostprocessorValue("vectorpostprocessor", getParam<std::string>("vector"))),
    _rank(getParam<processor_id_type>("rank")),
    _gold(getParam<std::vector<double>>("gold"))
{
}

void
TestCSVReader::execute()
{
  if (_communicator.rank() == _rank)
    if (_gold != _vpp_data)
      mooseError("The supplied gold data does not match the VPP data on the given rank.");
}
