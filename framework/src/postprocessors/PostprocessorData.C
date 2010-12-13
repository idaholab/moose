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
PostprocessorData::storeValue(const std::string & name, Real value)
{
  _values[name] = value;
}

void
PostprocessorData::addData(const std::string & name, Real value, Real time)
{
  _output_table.addData(name, value, time);
}

void
PostprocessorData::printTable(std::ostream & out)
{
  _output_table.printTable(out);
}

void
PostprocessorData::printTable(const std::string & file_name)
{
  _output_table.printTable(file_name);
}

void
PostprocessorData::printCSV(const std::string & file_name)
{
  _output_table.printCSV(file_name);
}

void
PostprocessorData::printEnsight(const std::string & file_name)
{
  _output_table.printEnsight(file_name);
}

void
PostprocessorData::makeGnuplot(const std::string & file_name, const std::string & format)
{
  _output_table.makeGnuplot(file_name, format);
}

void
PostprocessorData::writeExodus( ExodusII_IO * ex_out, Real time )
{
  _output_table.writeExodus( ex_out, time );
}
