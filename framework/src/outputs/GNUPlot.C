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
#include "GNUPlot.h"

template<>
InputParameters validParams<GNUPlot>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<TableOutputter>();

  // Set an enum for the possible file extensions
  MooseEnum ext("png ps gif", "png", "GNU plot file extension");
  params.addParam<MooseEnum>("extension", ext, "GUN plot file extension");

  // Suppress unused parameters
  params.suppressParameter<unsigned int>("padding");

  return params;
}

GNUPlot::GNUPlot(const std::string & name, InputParameters & parameters) :
    TableOutputter(name, parameters),
    _extension(getParam<MooseEnum>("extension"))
{
}

GNUPlot::~GNUPlot()
{
}

std::string
GNUPlot::filename()
{
  return _file_base;
}

void
GNUPlot::output()
{
  // Call the base class output (populates tables)
  TableOutputter::output();

  // Print the table containing all the data to a file
  if (!_all_data_table.empty())
    _all_data_table.makeGnuplot(filename(), _extension);
}
