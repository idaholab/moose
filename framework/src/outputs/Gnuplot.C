//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Gnuplot.h"

template <>
InputParameters
validParams<Gnuplot>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<TableOutput>();

  // Set an enum for the possible file extensions
  MooseEnum ext("png ps gif", "png", true);
  params.addParam<MooseEnum>("extension", ext, "GNU plot file extension");

  return params;
}

Gnuplot::Gnuplot(const InputParameters & parameters)
  : TableOutput(parameters), _extension(getParam<MooseEnum>("extension"))
{
}

std::string
Gnuplot::filename()
{
  return _file_base;
}

void
Gnuplot::output(const ExecFlagType & type)
{
  // Call the base class output (populates tables)
  TableOutput::output(type);

  // Print the table containing all the data to a file
  if (!_all_data_table.empty())
    _all_data_table.makeGnuplot(filename(), _extension);

  // Update the file number
  _file_num++;
}
