#include "FormatedTable.h"

#include <iomanip>

FormatedTable::FormatedTable()
{}

void
FormatedTable::addData(const std::string & name, Real value, Real time)
{
  _data[time][name] = value;
  _column_names.insert(name);
}

void
FormatedTable::print(std::ostream & out)
{
  std::map<Real, std::map<std::string, Real> >::iterator i;
  std::map<std::string, Real>::iterator j;
  std::set<std::string>::iterator header;

  /**
   * Print out the header row
   */
  out << std::setw(8) << "time";
  for (header = _column_names.begin(); header != _column_names.end(); ++header)
  {
    out << std::setw(8) << *header;
  }
  out << "\n";
  
  for (i = _data.begin(); i != _data.end(); ++i)
  {
    out << std::setw(8) << i->first;
    for (header = _column_names.begin(); header != _column_names.end(); ++header)
    {
      std::map<std::string, Real> &tmp = i->second;
      out << std::setw(8) << tmp[*header];
    }
    out << "\n";
  }
}
