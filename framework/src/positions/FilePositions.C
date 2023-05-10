//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FilePositions.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", FilePositions);

InputParameters
FilePositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Import positions from one or more files.");
  params.addRequiredParam<std::vector<FileName>>("files",
                                                 "Text file(s) with the positions, one per line");

  // Input from file should not be re-ordered
  params.set<bool>("auto_sort") = false;
  // File is loaded on all processes
  params.set<bool>("auto_broadcast") = false;

  return params;
}

FilePositions::FilePositions(const InputParameters & parameters) : Positions(parameters)
{
  const auto & positions_files = getParam<std::vector<FileName>>("files");
  _positions_2d.resize(positions_files.size());

  // Copied from MultiApp.C
  for (const auto p_file_it : index_range(positions_files))
  {
    const std::string positions_file = positions_files[p_file_it];
    MooseUtils::DelimitedFileReader file(positions_file, &_communicator);
    file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
    file.read();

    const std::vector<Point> & data = file.getDataAsPoints();
    for (const auto & d : data)
    {
      _positions.push_back(d);
      _positions_2d[p_file_it].push_back(d);
    }
  }
}
