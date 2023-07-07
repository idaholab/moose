//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSVFileTimes.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", CSVFileTimes);

InputParameters
CSVFileTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addClassDescription("Import times from one or more files.");
  params.addRequiredParam<std::vector<FileName>>("files",
                                                 "Text file(s) with the times, one per line");
  params.addParam<unsigned int>("time_column_index", 0, "Index for the column with the time");
  // File is loaded on all processes
  params.set<bool>("auto_broadcast") = false;

  return params;
}

CSVFileTimes::CSVFileTimes(const InputParameters & parameters)
  : Times(parameters), _time_column_index(getParam<unsigned int>("time_column_index"))
{
  const auto & times_files = getParam<std::vector<FileName>>("files");

  // Copied from MultiApp.C
  for (const auto p_file_it : index_range(times_files))
  {
    const std::string times_file = times_files[p_file_it];
    MooseUtils::DelimitedFileReader file(times_file, &_communicator);
    file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::COLUMNS);
    file.read();

    const auto & data = file.getData();
    for (const auto & d : data[_time_column_index])
      _times.push_back(d);
  }
}
