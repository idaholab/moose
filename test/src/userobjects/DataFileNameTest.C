//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DataFileNameTest.h"

registerMooseObject("MooseTestApp", DataFileNameTest);

InputParameters
DataFileNameTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<FileName>("data_file", "Data file to look up");
  return params;
}

DataFileNameTest::DataFileNameTest(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  // a data file name supplied through an input parameter
  mooseInfo(getDataFileName("data_file"));

  // a hard coded data file name
  mooseInfo(getDataFileNameByName("README.md"));
}
