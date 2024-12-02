//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DataFileNameTest.h"

#include "ExecutablePath.h"

#include <filesystem>

registerMooseObject("MooseTestApp", DataFileNameTest);

InputParameters
DataFileNameTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<DataFileName>("data_file",
                                "Data file to look up that is loaded from the params)");
  params.addParam<std::string>("data_file_by_path", "Data file to look up by path (not param)");
  params.addParam<DataFileName>("data_file_deprecated",
                                "Data file to look up that is loaded from the deprecated method");
  params.addParam<std::string>("data_file_name_by_name",
                               "Data file to look up using the deprecated getDataFileNameByName");
  return params;
}

DataFileNameTest::DataFileNameTest(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  const auto print_path = [&](const std::string & name, const std::string & path)
  {
    _console << name
             << "_relative=" << std::filesystem::relative(path, std::filesystem::current_path())
             << std::endl;
    _console << name << "_relative_to_binary="
             << std::filesystem::relative(path, Moose::getExecutablePath()) << std::endl;
  };
  if (isParamSetByUser("data_file"))
    print_path("data_file", getParam<DataFileName>("data_file"));
  if (isParamSetByUser("data_file_by_path"))
    print_path("data_file_by_path", getDataFilePath(getParam<std::string>("data_file_by_path")));
  if (isParamSetByUser("data_file_deprecated"))
    print_path("data_file_deprecated", getDataFileName("data_file_deprecated"));
  if (isParamSetByUser("data_file_name_by_name"))
    print_path("data_file_name_by_name",
               getDataFileNameByName(getParam<std::string>("data_file_name_by_name")));
}
