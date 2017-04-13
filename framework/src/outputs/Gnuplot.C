/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
