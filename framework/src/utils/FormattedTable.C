#include "FormattedTable.h"

#include <iomanip>
#include <iterator>

const unsigned short FormattedTable::_column_width = 15;

FormattedTable::FormattedTable()
  : _stream_open(false)
{}

FormattedTable::FormattedTable(const FormattedTable &o)
{

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

void
FormattedTable::addData(const std::string & name, Real value, Real time)
{
  _data[time][name] = value;
  _column_names.insert(name);
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
