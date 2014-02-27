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

#include <iomanip>
#include <iterator>

// Used for terminal width
#include <sys/ioctl.h>
#include <cstdlib>

const unsigned short FormattedTable::_column_width = 15;
const unsigned short FormattedTable::_min_pps_width = 40;

template<>
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

template<>
void
dataLoad(std::istream & stream, FormattedTable & table, void * context)
{
  loadHelper(stream, table._data, context);

  loadHelper(stream, table._column_names, context);

  table._stream_open = false;

  loadHelper(stream, table._last_key, context);
}

FormattedTable::FormattedTable() :
    _stream_open(false),
    _last_key(-1)
{}

FormattedTable::FormattedTable(const FormattedTable &o) :
    _column_names(o._column_names),
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
FormattedTable::printOmittedRow(std::ostream & out, std::map<std::string, unsigned short> & col_widths,
                                std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end) const
{
  printNoDataRow(':', ' ', out, col_widths, col_begin, col_end);
}

void
FormattedTable::printRowDivider(std::ostream & out, std::map<std::string, unsigned short> & col_widths,
                                std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end) const
{
  printNoDataRow('+', '-', out, col_widths, col_begin, col_end);
}

void
FormattedTable::printNoDataRow(char intersect_char, char fill_char,
                               std::ostream & out, std::map<std::string, unsigned short> & col_widths,
                               std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end) const
{
  out.fill(fill_char);
  out << std::right << intersect_char << std::setw(_column_width+2) << intersect_char;
  for (std::set<std::string>::iterator header = col_begin; header != col_end; ++header)
  {
    out << std::setw(col_widths[*header]+2) << intersect_char;
  }
  out << "\n";

  // Clear the fill character
  out.fill(' ');
}

void
FormattedTable::printTable(const std::string & file_name)
{
  // We only want to do file I/O on processor zero
  if (libMesh::processor_id() != 0)
    return;

  if (!_stream_open)
  {
    _output_file.open(file_name.c_str(), std::ios::trunc);
    _stream_open = true;
  }
  printTable(_output_file);
}

void
FormattedTable::printTable(std::ostream & out, unsigned int last_n_entries)
{
  printTable(out, last_n_entries, MooseEnum("ENVIRONMENT=-1", "ENVIRONMENT"));
}

void
FormattedTable::printTable(std::ostream & out, unsigned int last_n_entries, const MooseEnum & suggested_term_width)
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
      col_widths[*col_it] = col_it->length() > _column_width ? col_it->length()+1 : _column_width;

      curr_width += col_widths[*col_it] + 3;
      ++col_it;
      ++cols_in_group;
    }
    if (col_it != col_end && cols_in_group >= 2)
    {
      //curr_width -= col_widths[*curr_end];
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
FormattedTable::printTablePiece(std::ostream & out, unsigned int last_n_entries, std::map<std::string, unsigned short> & col_widths,
                                std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end)
{
  std::map<Real, std::map<std::string, Real> >::iterator i;
  std::set<std::string>::iterator header;

  /**
   * Print out the header row
   */
  printRowDivider(out, col_widths, col_begin, col_end);
  out << "|" << std::setw(_column_width) << std::left << " time" << " |";
  for (header = col_begin; header != col_end; ++header)
  {
    out << " " << std::setw(col_widths[*header])  <<  *header << "|";
  }
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

    for (int counter=0; counter < static_cast<int>(_data.size() - last_n_entries); ++counter)
      ++i;
  }
  // Now print the remaining data rows
  for ( ; i != _data.end(); ++i)
  {
    out << "|" << std::right << std::setw(_column_width) << i->first << " |";
    for (header = col_begin; header != col_end; ++header)
    {
      std::map<std::string, Real> &tmp = i->second;
      out << std::setw(col_widths[*header]) << tmp[*header] << " |";
    }
    out << "\n";
  }

  printRowDivider(out, col_widths, col_begin, col_end);
}

void
FormattedTable::printCSV(const std::string & file_name, int interval)
{
  std::map<Real, std::map<std::string, Real> >::iterator i;
  std::set<std::string>::iterator header;

  // We only want to do file I/O on processor zero
  if (libMesh::processor_id() != 0)
    return;

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

  int counter = 0;
  for (i = _data.begin(); i != _data.end(); ++i)
  {
    if (counter++ % interval == 0)
    {
      _output_file << i->first;
      for (header = _column_names.begin(); header != _column_names.end(); ++header)
      {
        std::map<std::string, Real> &tmp = i->second;
        _output_file << "," << std::setprecision(14) << tmp[*header];
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
  const std::string before_ext      = "\nset output 'all";
  const std::string after_ext       = "'\nset title 'All Postprocessors'\nset xlabel 'time'\nset ylabel 'values'\nplot";
}

void
FormattedTable::makeGnuplot(const std::string & base_file, const std::string & format)
{
  // TODO: run this once at end of simulation, right now it runs every iteration
  // TODO: do I need to be more careful escaping column names?
  // Note: open and close the files each time, having open files may mess with gnuplot
  std::map<Real, std::map<std::string, Real> >::iterator i;
  std::set<std::string>::iterator header;

  // supported filetypes: ps, png
  std::string extension, terminal;
  if (format == "png")
  {
    extension = ".png"; terminal = "png";
  }
  else if (format == "ps")
  {
    extension = ".ps";  terminal = "postscript";
  }
  else if (format == "gif")
  {
    extension = ".gif"; terminal = "gif";
  }
  else
  {
    mooseError("gnuplot format \"" + format + "\" is not supported.");
  }

  // Write the data to disk
  std::string dat_name = base_file + ".dat";
  std::ofstream datfile;
  datfile.open(dat_name.c_str(), std::ios::trunc | std::ios::out);

  datfile << "# time";
  for (header = _column_names.begin(); header != _column_names.end(); ++header)
    datfile << '\t' << *header;
  datfile << '\n';

  for (i = _data.begin(); i != _data.end(); ++i)
  {
    datfile << i->first;
    for (header = _column_names.begin(); header != _column_names.end(); ++header)
    {
      std::map<std::string, Real> &tmp = i->second;
      datfile << '\t' << tmp[*header];
    }
    datfile << '\n';
  }
  datfile.flush();
  datfile.close();

  // Write the gnuplot script
  std::string gp_name = base_file + ".gp";
  std::ofstream gpfile;
  gpfile.open(gp_name.c_str(), std::ios::trunc | std::ios::out);

  gpfile << gnuplot::before_terminal << terminal << gnuplot::before_ext << extension << gnuplot::after_ext;

  // plot all postprocessors in one plot
  int column = 2;
  for (header = _column_names.begin(); header != _column_names.end(); ++header)
  {
    gpfile << " '" << dat_name << "' using 1:" << column << " title '" << *header << "' with linespoints";
    column++;
    if ( column - 2 < (int) _column_names.size() )
      gpfile << ", \\\n";
  }
  gpfile << "\n\n";

  // plot the postprocessors individually
  column = 2;
  for (header = _column_names.begin(); header != _column_names.end(); ++header)
  {
    gpfile << "set output '" << *header << extension << "'\n";
    gpfile << "set ylabel '" << *header << "'\n";
    gpfile << "plot '" << dat_name << "' using 1:" << column << " title '" << *header << "' with linespoints\n\n";
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
    char *pps_width = std::getenv("MOOSE_PPS_WIDTH");
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
    catch(...)
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
  return MooseEnum("ENVIRONMENT=-1, AUTO=0, 80=80, 120=120, 160=160", "ENVIRONMENT", true);
}
