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
  if (_values.find(name) == _values.end())
    mooseError("No Data found for name: " + name);
  
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

