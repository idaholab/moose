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

// Moose includes
#include "CSV.h"

template<>
InputParameters validParams<CSV>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<TableOutputter>();
  params += validParams<FileOutputInterface>();

  return params;
}

CSV::CSV(const std::string & name, InputParameters & parameters) :
    TableOutputter(name, parameters),
    FileOutputInterface(name, parameters)
{
}

CSV::~CSV()
{
}

std::string
CSV::filename()
{
  return _file_base + ".csv";
}

void
CSV::output()
{
  // Call the base class output (populates tables)
  TableOutputter::output();

  // Print the table containing all the data to a file
  if (!_all_data_table.empty())
    _all_data_table.printCSV(filename());
}
