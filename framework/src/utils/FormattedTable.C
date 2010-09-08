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

#include "FormattedTable.h"
#include "Moose.h"

#include <iomanip>
#include <iterator>

const unsigned short FormattedTable::_column_width = 15;

FormattedTable::FormattedTable()
  : _stream_open(false),
    _last_key(-1)
{}

FormattedTable::FormattedTable(const FormattedTable &o)
  : _column_names(o._column_names),
    _stream_open(o._stream_open),
    _last_key(o._last_key)
{
  if (_stream_open)
    mooseError ("Copying a FormattedTable with an open stream is not supported");

  std::map<Real, std::map<std::string, Real> >::const_iterator it = o._data.begin();
  
  for ( ; it != o._data.end(); ++it)
    _data[it->first] = it->second;
}

FormattedTable::~FormattedTable()
{
  if (_stream_open)
  {
    _output_file.flush();
    _output_file.close();
    _stream_open = false;
  }
}

bool
FormattedTable::empty() const
{
  return _last_key == -1 ? true : false;
}

void
FormattedTable::addData(const std::string & name, Real value, Real time)
{
  _data[time][name] = value;
  _column_names.insert(name);
  _last_key = time;
}

Real &
FormattedTable::getLastData(const std::string & name)
{
  mooseAssert(_last_key != -1, "No Data stored in the FormattedTable");

  std::map<std::string, Real>::iterator it = (_data[_last_key]).find(name);
  if (it != (_data[_last_key]).end())
    return it->second;

  mooseError("No Data found for name: " + name);
}


void
FormattedTable::printRowDivider(std::ostream & out)
{
  std::string divider(_column_width+1, '-');
  
  out << "+";
  for (unsigned int i = 0; i < _column_names.size() + 1; ++i)
  {
    out << divider << "+";
  }
  out << "\n";
}

void
FormattedTable::print_table(const std::string & file_name)
{
  if (!_stream_open)
    _output_file.open(file_name.c_str(), std::ios::trunc);
  print_table(_output_file);
}


void
FormattedTable::print_table(std::ostream & out)
{
  std::map<Real, std::map<std::string, Real> >::iterator i;
  std::set<std::string>::iterator header;

  /**
   * Print out the header row
   */
  printRowDivider(out);
  out << "|" << std::setw(_column_width) << std::left << " time" << " |";
  for (header = _column_names.begin(); header != _column_names.end(); ++header)
  {
    out << " " << std::setw(_column_width)  <<  *header << "|";
  }
  out << "\n";
  printRowDivider(out);
  
  for (i = _data.begin(); i != _data.end(); ++i)
  {
    out << "|" << std::right << std::setw(_column_width) << i->first << " |";
    for (header = _column_names.begin(); header != _column_names.end(); ++header)
    {
      std::map<std::string, Real> &tmp = i->second;
      out << std::setw(_column_width) << tmp[*header] << " |";
    }
    out << "\n";
  }
  
  printRowDivider(out);
}

void
FormattedTable::print_csv(const std::string & file_name)
{
  std::map<Real, std::map<std::string, Real> >::iterator i;
  std::set<std::string>::iterator header;
  
  if (!_stream_open)
  {
    _output_file.open(file_name.c_str(), std::ios::trunc | std::ios::out);
    _stream_open = true;
  }

  _output_file.seekp(0, std::ios::beg);
  _output_file << "time";
  for (header = _column_names.begin(); header != _column_names.end(); ++header)
  {
    _output_file << "," << *header;
  }
  _output_file << "\n";
  
  for (i = _data.begin(); i != _data.end(); ++i)
  {
    _output_file << i->first;
    for (header = _column_names.begin(); header != _column_names.end(); ++header)
    {
      std::map<std::string, Real> &tmp = i->second;
      _output_file << "," << tmp[*header];
    }
    _output_file << "\n";
  }
  _output_file << "\n";
}
