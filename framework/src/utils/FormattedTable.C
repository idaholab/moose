//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FormattedTable.h"
#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/exodusII_io.h"

#include <iomanip>
#include <iterator>

// Used for terminal width
#ifndef __WIN32__
#include <sys/ioctl.h>
#endif
#include <cstdlib>

const unsigned short FormattedTable::_column_width = 15;
const unsigned short FormattedTable::_min_pps_width = 40;

const unsigned short DEFAULT_CSV_PRECISION = 14;
const std::string DEFAULT_CSV_DELIMITER = ",";

template <>
void
dataStore(std::ostream & stream, FormattedTable & table, void * context)
{
  table.fillEmptyValues();
  storeHelper(stream, table._data, context);
  storeHelper(stream, table._align_widths, context);
  storeHelper(stream, table._column_names, context);
  storeHelper(stream, table._output_row_index, context);
  storeHelper(stream, table._headers_output, context);
}

template <>
void
dataLoad(std::istream & stream, FormattedTable & table, void * context)
{
  loadHelper(stream, table._data, context);
  loadHelper(stream, table._align_widths, context);
  loadHelper(stream, table._column_names, context);
  loadHelper(stream, table._output_row_index, context);
  loadHelper(stream, table._headers_output, context);
}

template <>
void
dataStore(std::ostream & stream, std::shared_ptr<TableValueBase> & value_base, void * context)
{
  value_base->store(stream, context);
}

template <>
void
dataLoad(std::istream & stream, std::shared_ptr<TableValueBase> & value_base, void * context)
{
  std::string type;
  dataLoad(stream, type, context);
  if (type == typeid(bool).name())
    TableValue<bool>::load(stream, value_base, context);

  else if (type == typeid(unsigned short int).name())
    TableValue<unsigned short int>::load(stream, value_base, context);

  else if (type == typeid(unsigned int).name())
    TableValue<unsigned int>::load(stream, value_base, context);

  else if (type == typeid(unsigned long int).name())
    TableValue<unsigned long int>::load(stream, value_base, context);

  else if (type == typeid(unsigned long long int).name())
    TableValue<unsigned long long int>::load(stream, value_base, context);

  else if (type == typeid(short int).name())
    TableValue<short int>::load(stream, value_base, context);

  else if (type == typeid(int).name())
    TableValue<int>::load(stream, value_base, context);

  else if (type == typeid(long int).name())
    TableValue<long int>::load(stream, value_base, context);

  else if (type == typeid(long long int).name())
    TableValue<long long int>::load(stream, value_base, context);

  else if (type == typeid(float).name())
    TableValue<float>::load(stream, value_base, context);

  else if (type == typeid(double).name())
    TableValue<double>::load(stream, value_base, context);

  else if (type == typeid(long double).name())
    TableValue<long double>::load(stream, value_base, context);

  else if (type == typeid(char).name())
    TableValue<char>::load(stream, value_base, context);

  else if (type == typeid(char *).name())
    TableValue<char *>::load(stream, value_base, context);

  else if (type == typeid(std::string).name())
    TableValue<std::string>::load(stream, value_base, context);

  else
    mooseError("Unsupported table value type ", demangle(type.c_str()));
}

void
FormattedTable::close()
{
  if (!_output_file.is_open())
    return;
  _output_file.flush();
  _output_file.close();
  _output_file_name = "";
}

void
FormattedTable::open(const std::string & file_name)
{
  if (_output_file.is_open() && _output_file_name == file_name)
    return;
  close();
  _output_file_name = file_name;

  std::ios_base::openmode open_flags = std::ios::out;
  if (_append)
    open_flags |= std::ios::app;
  else
  {
    open_flags |= std::ios::trunc;
    _output_row_index = 0;
    _headers_output = false;
  }

  _output_file.open(file_name.c_str(), open_flags);
  if (_output_file.fail())
    mooseError("Unable to open file ", file_name);
}

FormattedTable::FormattedTable()
  : _output_row_index(0),
    _headers_output(false),
    _append(false),
    _output_time(true),
    _csv_delimiter(DEFAULT_CSV_DELIMITER),
    _csv_precision(DEFAULT_CSV_PRECISION)
{
}

FormattedTable::FormattedTable(const FormattedTable & o)
  : _column_names(o._column_names),
    _output_file_name(""),
    _output_row_index(o._output_row_index),
    _headers_output(o._headers_output),
    _append(o._append),
    _output_time(o._output_time),
    _csv_delimiter(o._csv_delimiter),
    _csv_precision(o._csv_precision),
    _column_names_unsorted(o._column_names_unsorted)
{
  if (_output_file.is_open())
    mooseError("Copying a FormattedTable with an open stream is not supported");

  for (const auto & it : o._data)
    _data.emplace_back(it.first, it.second);
}

FormattedTable::~FormattedTable() { close(); }

bool
FormattedTable::empty() const
{
  return _data.empty();
}

void
FormattedTable::append(bool append_existing_file)
{
  _append = append_existing_file;
}

void
FormattedTable::addRow(Real time)
{
  _data.emplace_back(time, std::map<std::string, std::shared_ptr<TableValueBase>>());
}

Real
FormattedTable::getLastTime()
{
  mooseAssert(!empty(), "No Data stored in the FormattedTable");
  return _data.rbegin()->first;
}

void
FormattedTable::printOmittedRow(std::ostream & out,
                                std::map<std::string, unsigned short> & col_widths,
                                std::vector<std::string>::iterator & col_begin,
                                std::vector<std::string>::iterator & col_end) const
{
  printNoDataRow(':', ' ', out, col_widths, col_begin, col_end);
}

void
FormattedTable::printRowDivider(std::ostream & out,
                                std::map<std::string, unsigned short> & col_widths,
                                std::vector<std::string>::iterator & col_begin,
                                std::vector<std::string>::iterator & col_end) const
{
  printNoDataRow('+', '-', out, col_widths, col_begin, col_end);
}

void
FormattedTable::printNoDataRow(char intersect_char,
                               char fill_char,
                               std::ostream & out,
                               std::map<std::string, unsigned short> & col_widths,
                               std::vector<std::string>::iterator & col_begin,
                               std::vector<std::string>::iterator & col_end) const
{
  out.fill(fill_char);
  out << std::right << intersect_char << std::setw(_column_width + 2) << intersect_char;
  for (auto header_it = col_begin; header_it != col_end; ++header_it)
    out << std::setw(col_widths[*header_it] + 2) << intersect_char;
  out << "\n";

  // Clear the fill character
  out.fill(' ');
}

void
FormattedTable::printTable(const std::string & file_name)
{
  open(file_name);
  printTable(_output_file);
}

void
FormattedTable::printTable(std::ostream & out, unsigned int last_n_entries)
{
  printTable(out, last_n_entries, MooseEnum("ENVIRONMENT=-1", "ENVIRONMENT"));
}

void
FormattedTable::printTable(std::ostream & out,
                           unsigned int last_n_entries,
                           const MooseEnum & suggested_term_width)
{
  unsigned short term_width;

  if (suggested_term_width == "ENVIRONMENT")
    term_width = getTermWidth(true);
  else if (suggested_term_width == "AUTO")
    term_width = getTermWidth(false);
  else
    term_width = MooseUtils::stringToInteger(suggested_term_width);

  if (term_width < _min_pps_width)
    term_width = _min_pps_width;

  std::vector<std::string>::iterator col_it = _column_names.begin();
  std::vector<std::string>::iterator col_end = _column_names.end();

  std::vector<std::string>::iterator curr_begin = col_it;
  std::vector<std::string>::iterator curr_end;
  while (col_it != col_end)
  {
    std::map<std::string, unsigned short> col_widths;
    unsigned int curr_width = _column_width + 4;
    unsigned int cols_in_group = 0;
    while (curr_width < term_width && col_it != col_end)
    {
      curr_end = col_it;
      col_widths[*col_it] = col_it->length() > _column_width ? col_it->length() + 1 : _column_width;

      curr_width += col_widths[*col_it] + 3;
      ++col_it;
      ++cols_in_group;
    }
    if (col_it != col_end && cols_in_group >= 2)
    {
      // curr_width -= col_widths[*curr_end];
      col_widths.erase(*curr_end);
      col_it = curr_end;
    }
    else
      curr_end = col_it;

    printTablePiece(out, last_n_entries, col_widths, curr_begin, curr_end);
    curr_begin = curr_end;
  }
}

void
FormattedTable::printTablePiece(std::ostream & out,
                                unsigned int last_n_entries,
                                std::map<std::string, unsigned short> & col_widths,
                                std::vector<std::string>::iterator & col_begin,
                                std::vector<std::string>::iterator & col_end)
{
  fillEmptyValues();
  /**
   * Print out the header row
   */
  printRowDivider(out, col_widths, col_begin, col_end);
  out << "|" << std::setw(_column_width) << std::left << " time"
      << " |";
  for (auto header_it = col_begin; header_it != col_end; ++header_it)
    out << " " << std::setw(col_widths[*header_it]) << *header_it << "|";
  out << "\n";
  printRowDivider(out, col_widths, col_begin, col_end);

  auto data_it = _data.begin();
  if (last_n_entries)
  {
    if (_data.size() > last_n_entries)
    {
      // Print a blank row to indicate that values have been ommited
      printOmittedRow(out, col_widths, col_begin, col_end);

      // Jump to the right place in the vector
      data_it += _data.size() - last_n_entries;
    }
  }
  // Now print the remaining data rows
  for (; data_it != _data.end(); ++data_it)
  {
    out << "|" << std::right << std::setw(_column_width) << std::scientific << data_it->first
        << " |";
    for (auto header_it = col_begin; header_it != col_end; ++header_it)
    {
      auto & tmp = data_it->second;
      out << std::setw(col_widths[*header_it]) << *tmp[*header_it] << " |";
    }
    out << "\n";
  }

  printRowDivider(out, col_widths, col_begin, col_end);
}

void
FormattedTable::printCSV(const std::string & file_name, int interval, bool align)
{
  fillEmptyValues();

  open(file_name);

  if (_output_row_index == 0)
  {
    /**
     * When the alignment option is set to true, the widths of the columns needs to be computed
     * based on longest of the column name of the data supplied. This is done here by creating a
     * map
     * of the widths for each of the columns, including time
     */
    if (align)
    {
      // Set the initial width to the names of the columns
      _align_widths["time"] = 4;

      for (const auto & col_name : _column_names)
        _align_widths[col_name] = col_name.size();

      // Loop through the various times
      for (const auto & it : _data)
      {
        // Update the time _align_width
        {
          std::ostringstream oss;
          oss << std::setprecision(_csv_precision) << it.first;
          unsigned int w = oss.str().size();
          _align_widths["time"] = std::max(_align_widths["time"], w);
        }

        // Loop through the data for the current time and update the _align_widths
        for (const auto & jt : it.second)
        {
          std::ostringstream oss;
          oss << std::setprecision(_csv_precision) << *jt.second;
          unsigned int w = oss.str().size();
          _align_widths[jt.first] = std::max(_align_widths[jt.first], w);
        }
      }
    }

    // Output Header
    if (!_headers_output)
    {
      if (_output_time)
      {
        if (align)
          _output_file << std::setw(_align_widths["time"]) << "time";
        else
          _output_file << "time";
        _headers_output = true;
      }

      for (const auto & col_name : _column_names)
      {
        if (_headers_output)
          _output_file << _csv_delimiter;

        if (align)
          _output_file << std::right << std::setw(_align_widths[col_name]) << col_name;
        else
          _output_file << col_name;
        _headers_output = true;
      }
      _output_file << "\n";
    }
  }

  for (; _output_row_index < _data.size(); ++_output_row_index)
  {
    if (_output_row_index % interval == 0)
      printRow(_data[_output_row_index], align);
  }

  _output_file.flush();
}

void
FormattedTable::printRow(
    std::pair<Real, std::map<std::string, std::shared_ptr<TableValueBase>>> & row_data, bool align)
{
  bool first = true;

  if (_output_time)
  {
    if (align)
      _output_file << std::setprecision(_csv_precision) << std::right
                   << std::setw(_align_widths["time"]) << row_data.first;
    else
      _output_file << std::setprecision(_csv_precision) << row_data.first;
    first = false;
  }

  for (const auto & col_name : _column_names)
  {
    std::map<std::string, std::shared_ptr<TableValueBase>> & tmp = row_data.second;

    if (!first)
      _output_file << _csv_delimiter;
    else
      first = false;

    if (align)
      _output_file << std::setprecision(_csv_precision) << std::right
                   << std::setw(_align_widths[col_name]) << *tmp[col_name];
    else
      _output_file << std::setprecision(_csv_precision) << *tmp[col_name];
  }
  _output_file << "\n";
}

// const strings that the gnuplot generator needs
namespace gnuplot
{
const std::string before_terminal = "set terminal ";
const std::string before_ext = "\nset output 'all";
const std::string after_ext =
    "'\nset title 'All Postprocessors'\nset xlabel 'time'\nset ylabel 'values'\nplot";
}

void
FormattedTable::makeGnuplot(const std::string & base_file, const std::string & format)
{
  fillEmptyValues();

  // TODO: run this once at end of simulation, right now it runs every iteration
  // TODO: do I need to be more careful escaping column names?
  // Note: open and close the files each time, having open files may mess with gnuplot

  // supported filetypes: ps, png
  std::string extension, terminal;
  if (format == "png")
  {
    extension = ".png";
    terminal = "png";
  }

  else if (format == "ps")
  {
    extension = ".ps";
    terminal = "postscript";
  }

  else if (format == "gif")
  {
    extension = ".gif";
    terminal = "gif";
  }

  else
    mooseError("gnuplot format \"" + format + "\" is not supported.");

  // Write the data to disk
  std::string dat_name = base_file + ".dat";
  std::ofstream datfile;
  datfile.open(dat_name.c_str(), std::ios::trunc | std::ios::out);
  if (datfile.fail())
    mooseError("Unable to open file ", dat_name);

  datfile << "# time";
  for (const auto & col_name : _column_names)
    datfile << '\t' << col_name;
  datfile << '\n';

  for (auto & data_it : _data)
  {
    datfile << data_it.first;
    for (const auto & col_name : _column_names)
    {
      auto & tmp = data_it.second;
      datfile << '\t' << *tmp[col_name];
    }
    datfile << '\n';
  }
  datfile.flush();
  datfile.close();

  // Write the gnuplot script
  std::string gp_name = base_file + ".gp";
  std::ofstream gpfile;
  gpfile.open(gp_name.c_str(), std::ios::trunc | std::ios::out);
  if (gpfile.fail())
    mooseError("Unable to open file ", gp_name);

  gpfile << gnuplot::before_terminal << terminal << gnuplot::before_ext << extension
         << gnuplot::after_ext;

  // plot all postprocessors in one plot
  int column = 2;
  for (const auto & col_name : _column_names)
  {
    gpfile << " '" << dat_name << "' using 1:" << column << " title '" << col_name
           << "' with linespoints";
    column++;
    if (column - 2 < static_cast<int>(_column_names.size()))
      gpfile << ", \\\n";
  }
  gpfile << "\n\n";

  // plot the postprocessors individually
  column = 2;
  for (const auto & col_name : _column_names)
  {
    gpfile << "set output '" << col_name << extension << "'\n";
    gpfile << "set ylabel '" << col_name << "'\n";
    gpfile << "plot '" << dat_name << "' using 1:" << column << " title '" << col_name
           << "' with linespoints\n\n";
    column++;
  }

  gpfile.flush();
  gpfile.close();
}

void
FormattedTable::clear()
{
  _data.clear();
  _output_file.close();
  _output_row_index = 0;
}

void
FormattedTable::fillEmptyValues()
{
  for (auto & it : _data)
    for (const auto & col_name : _column_names)
      if (!it.second[col_name])
        it.second[col_name] =
            std::dynamic_pointer_cast<TableValueBase>(std::make_shared<TableValue<char>>('0'));
}

unsigned short
FormattedTable::getTermWidth(bool use_environment) const
{
#ifndef __WIN32__
  struct winsize w;
#else
  struct
  {
    unsigned short ws_col;
  } w;
#endif
  /**
   * Initialize the value we intend to populate just in case
   * the system call fails
   */
  w.ws_col = std::numeric_limits<unsigned short>::max();

  if (use_environment)
  {
    char * pps_width = std::getenv("MOOSE_PPS_WIDTH");
    if (pps_width != NULL)
    {
      std::stringstream ss(pps_width);
      ss >> w.ws_col;
    }
  }
  // Default to AUTO if no environment variable was set
  if (w.ws_col == std::numeric_limits<unsigned short>::max())
  {
#ifndef __WIN32__
    try
    {
      ioctl(0, TIOCGWINSZ, &w);
    }
    catch (...)
#endif
    {
    }
  }

  // Something bad happened, make sure we have a sane value
  // 132 seems good for medium sized screens, and is available as a GNOME preset
  if (w.ws_col == std::numeric_limits<unsigned short>::max())
    w.ws_col = 132;

  return w.ws_col;
}

MooseEnum
FormattedTable::getWidthModes()
{
  return MooseEnum("ENVIRONMENT=-1 AUTO=0 80=80 120=120 160=160", "ENVIRONMENT", true);
}

void
FormattedTable::sortColumns()
{
  if (_column_names_unsorted)
  {
    std::sort(_column_names.begin(), _column_names.end());
    _column_names_unsorted = false;
  }
}

std::ostream &
operator<<(std::ostream & os, const TableValueBase & value)
{
  value.print(os);
  return os;
}
