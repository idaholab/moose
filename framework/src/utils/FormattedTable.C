#include "FormattedTable.h"

#include <iomanip>

const unsigned short FormattedTable::_column_width = 15;

FormattedTable::FormattedTable()
{}

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
FormattedTable::print(std::ostream & out)
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
