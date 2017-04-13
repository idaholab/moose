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
#include "MooseError.h"
#include "InfixIterator.h"

// libMesh includes
#include "libmesh/exodusII_io.h"

#include <iomanip>
#include <iterator>

// Used for terminal width
#include <sys/ioctl.h>
#include <cstdlib>

const unsigned short FormattedTable::_column_width = 15;
const unsigned short FormattedTable::_min_pps_width = 40;

template <>
void
dataStore(std::ostream & stream, FormattedTable & table, void * context)
{
  storeHelper(stream, table._data, context);
  storeHelper(stream, table._column_names, context);

  // Don't store these
  // _output_file
  // _stream_open

  storeHelper(stream, table._last_key, context);
}

template <>
void
dataLoad(std::istream & stream, FormattedTable & table, void * context)
{
  loadHelper(stream, table._data, context);

  loadHelper(stream, table._column_names, context);

  table._stream_open = false;
  // table.close();

  loadHelper(stream, table._last_key, context);
}

void
FormattedTable::close()
{
  if (!_stream_open)
    return;
  _output_file.flush();
  _output_file.close();
  _stream_open = false;
  _output_file_name = "";
}

void
FormattedTable::open(const std::string & file_name)
{
  if (!_stream_open && _output_file_name == file_name)
    return;
  close();
  _output_file_name = file_name;
  _output_file.open(file_name.c_str(), std::ios::trunc | std::ios::out);
  _stream_open = true;
}

FormattedTable::FormattedTable()
  : _stream_open(false), _last_key(-1), _output_time(true), _csv_delimiter(","), _csv_precision(14)
{
}

FormattedTable::FormattedTable(const FormattedTable & o)
  : _column_names(o._column_names),
    _output_file_name(""),
    _stream_open(o._stream_open),
    _last_key(o._last_key),
    _output_time(o._output_time),
    _csv_delimiter(","),
    _csv_precision(14)
{
  if (_stream_open)
    mooseError("Copying a FormattedTable with an open stream is not supported");

  for (const auto & it : o._data)
    _data[it.first] = it.second;
}

FormattedTable::~FormattedTable() { close(); }

bool
FormattedTable::empty() const
{
  return _last_key == -1;
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
  if (it == (_data[_last_key]).end())
    mooseError("No Data found for name: " + name);

  return it->second;
}

void
FormattedTable::printOmittedRow(std::ostream & out,
                                std::map<std::string, unsigned short> & col_widths,
                                std::set<std::string>::iterator & col_begin,
                                std::set<std::string>::iterator & col_end) const
{
  printNoDataRow(':', ' ', out, col_widths, col_begin, col_end);
}

void
FormattedTable::printRowDivider(std::ostream & out,
                                std::map<std::string, unsigned short> & col_widths,
                                std::set<std::string>::iterator & col_begin,
                                std::set<std::string>::iterator & col_end) const
{
  printNoDataRow('+', '-', out, col_widths, col_begin, col_end);
}

void
FormattedTable::printNoDataRow(char intersect_char,
                               char fill_char,
                               std::ostream & out,
                               std::map<std::string, unsigned short> & col_widths,
                               std::set<std::string>::iterator & col_begin,
                               std::set<std::string>::iterator & col_end) const
{
  out.fill(fill_char);
  out << std::right << intersect_char << std::setw(_column_width + 2) << intersect_char;
  for (std::set<std::string>::iterator header = col_begin; header != col_end; ++header)
    out << std::setw(col_widths[*header] + 2) << intersect_char;
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
    term_width = suggested_term_width;

  if (term_width < _min_pps_width)
    term_width = _min_pps_width;

  std::set<std::string>::iterator col_it = _column_names.begin();
  std::set<std::string>::iterator col_end = _column_names.end();

  std::set<std::string>::iterator curr_begin = col_it;
  std::set<std::string>::iterator curr_end;
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
                                std::set<std::string>::iterator & col_begin,
                                std::set<std::string>::iterator & col_end)
{
  std::map<Real, std::map<std::string, Real>>::iterator i;
  std::set<std::string>::iterator header;

  /**
   * Print out the header row
   */
  printRowDivider(out, col_widths, col_begin, col_end);
  out << "|" << std::setw(_column_width) << std::left << " time"
      << " |";
  for (header = col_begin; header != col_end; ++header)
    out << " " << std::setw(col_widths[*header]) << *header << "|";
  out << "\n";
  printRowDivider(out, col_widths, col_begin, col_end);

  /**
   * Skip over values that we don't want to see.
   * This step may be able to optimized if the table gets really big.  We could
   * iterate backwards from the end and create a new forward iterator from there.
   */
  i = _data.begin();
  if (last_n_entries)
  {
    if (_data.size() > last_n_entries)
      // Print a blank row to indicate that values have been ommited
      printOmittedRow(out, col_widths, col_begin, col_end);

    for (int counter = 0; counter < static_cast<int>(_data.size() - last_n_entries); ++counter)
      ++i;
  }
  // Now print the remaining data rows
  for (; i != _data.end(); ++i)
  {
    out << "|" << std::right << std::setw(_column_width) << std::scientific << i->first << " |";
    for (header = col_begin; header != col_end; ++header)
    {
      std::map<std::string, Real> & tmp = i->second;
      out << std::setw(col_widths[*header]) << tmp[*header] << " |";
    }
    out << "\n";
  }

  printRowDivider(out, col_widths, col_begin, col_end);
}

void
FormattedTable::printCSV(const std::string & file_name, int interval, bool align)
{
  open(file_name);
  _output_file.seekp(0, std::ios::beg);

  /* When the alignment option is set to true, the widths of the columns needs to be computed based
   * on
   * longest of the column name of the data supplied. This is done here by creating a map of the
   * widths for each of the columns, including time */
  std::map<std::string, unsigned int> width;
  if (align)
  {
    // Set the initial width to the names of the columns
    width["time"] = 4;
    for (const auto & col_name : _column_names)
      width[col_name] = col_name.size();

    // Loop through the various times
    for (const auto & it : _data)
    {
      // Update the time width
      {
        std::ostringstream oss;
        oss << std::setprecision(_csv_precision) << it.first;
        unsigned int w = oss.str().size();
        width["time"] = std::max(width["time"], w);
      }

      // Loop through the data for the current time and update the widths
      for (const auto & jt : it.second)
      {
        std::ostringstream oss;
        oss << std::setprecision(_csv_precision) << jt.second;
        unsigned int w = oss.str().size();
        width[jt.first] = std::max(width[jt.first], w);
      }
    }
  }

  { // Output Header
    bool first = true;

    if (_output_time)
    {
      if (align)
        _output_file << std::setw(width["time"]) << "time";
      else
        _output_file << "time";
      first = false;
    }

    for (const auto & col_name : _column_names)
    {
      if (!first)
        _output_file << _csv_delimiter;

      if (align)
        _output_file << std::right << std::setw(width[col_name]) << col_name;
      else
        _output_file << col_name;
      first = false;
    }
  }

  _output_file << "\n";

  int counter = 0;
  for (auto & i : _data)
  {
    if (counter++ % interval == 0)
    {
      bool first = true;

      if (_output_time)
      {
        if (align)
          _output_file << std::setprecision(_csv_precision) << std::right
                       << std::setw(width["time"]) << i.first;
        else
          _output_file << std::setprecision(_csv_precision) << i.first;
        first = false;
      }

      for (const auto & col_name : _column_names)
      {
        std::map<std::string, Real> & tmp = i.second;

        if (!first)
          _output_file << _csv_delimiter;
        else
          first = false;

        if (align)
          _output_file << std::setprecision(_csv_precision) << std::right
                       << std::setw(width[col_name]) << tmp[col_name];
        else
          _output_file << std::setprecision(_csv_precision) << tmp[col_name];
      }
      _output_file << "\n";
    }
  }
  _output_file << "\n";
  _output_file.flush();
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

  datfile << "# time";
  for (const auto & col_name : _column_names)
    datfile << '\t' << col_name;
  datfile << '\n';

  for (auto & i : _data)
  {
    datfile << i.first;
    for (const auto & col_name : _column_names)
    {
      std::map<std::string, Real> & tmp = i.second;
      datfile << '\t' << tmp[col_name];
    }
    datfile << '\n';
  }
  datfile.flush();
  datfile.close();

  // Write the gnuplot script
  std::string gp_name = base_file + ".gp";
  std::ofstream gpfile;
  gpfile.open(gp_name.c_str(), std::ios::trunc | std::ios::out);

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

  // Run the gnuplot script
  /* We aren't going to run gnuplot automatically

    if (!system(NULL))
      mooseError("No way to run gnuplot on this computer");

    std::string command = "gnuplot " + gp_name;
    if (system(command.c_str()))
      mooseError("gnuplot command failed");
  */
}

void
FormattedTable::clear()
{
  _data.clear();
}

unsigned short
FormattedTable::getTermWidth(bool use_environment) const
{
  struct winsize w;
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
  else
  {
    try
    {
      ioctl(0, TIOCGWINSZ, &w);
    }
    catch (...)
    {
      // Something bad happened, make sure we have a sane value
      w.ws_col = std::numeric_limits<unsigned short>::max();
    }
  }

  return w.ws_col;
}

MooseEnum
FormattedTable::getWidthModes()
{
  return MooseEnum("ENVIRONMENT=-1 AUTO=0 80=80 120=120 160=160", "ENVIRONMENT", true);
}
