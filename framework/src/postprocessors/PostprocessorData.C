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

#include "PostprocessorData.h"

//Moose includes
#include "MooseSystem.h"

PostprocessorData::PostprocessorData(MooseSystem & moose_system)
  :_moose_system(moose_system)
{}

bool
PostprocessorData::empty()
{
  return _output_table.empty();
}

PostprocessorValue &
PostprocessorData::getPostprocessorValue(const std::string & name)
{
  // TODO: do something smarter so we can have lazy binding like this... but still have good errors.
  /*
  if (_values.find(name) == _values.end())
    mooseError("No Data found for name: " + name);
  */
  
  return _values[name];
}

void
PostprocessorData::init(const std::string & name)
{
  _values[name] = 0.0;
}

void
PostprocessorData::addData(const std::string & name, Real value, Real time)
{
  _values[name] = value;
  _output_table.addData(name, value, time);
}

void
PostprocessorData::print_table(std::ostream & out)
{
  _output_table.print_table(out);
}

void
PostprocessorData::print_table(const std::string & file_name)
{
  _output_table.print_table(file_name);
}

void
PostprocessorData::print_csv(const std::string & file_name)
{
  _output_table.print_csv(file_name);
}

void
PostprocessorData::print_ensight(const std::string & file_name)
{
  _output_table.print_ensight(file_name);
}

void
PostprocessorData::make_gnuplot(const std::string & file_name, const std::string & format)
{
  _output_table.make_gnuplot(file_name, format);
}
