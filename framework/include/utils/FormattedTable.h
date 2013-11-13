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

#ifndef FORMATTEDTABLE_H
#define FORMATTEDTABLE_H

#include "Moose.h"
#include "MooseEnum.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/exodusII_io.h"

#include <string>
#include <map>
#include <set>
#include <ostream>
#include <fstream>

class FormattedTable
{
public:
  FormattedTable();

  FormattedTable(const FormattedTable &o);

  /**
   * The destructor is used to close the file handle
   */
  ~FormattedTable();

  /**
   * Returns a boolean value based on whether the FormattedTable contains data or not
   */
  bool empty() const;

  /**
   * Method for adding data to the output table.  The dependent variable is named "time"
   */
  void addData(const std::string & name, Real value, Real time);

  /**
   * Retrieve Data for last value of given name
   */
  Real & getLastData(const std::string & name);

  void clear();

  const std::map<Real, std::map<std::string, Real> > & getData() const { return _data; }

  /**
   * Methods for dumping the table to the stream - either by filename or by stream handle.  If
   * a filename is supplied opening and closing of the file is properly handled.  In the
   * screen version of the method, an optional parameters can be passed to print only the last
   * "n" entries.  A value of zero means don't skip any rows
   */
  void printTable(std::ostream & out, unsigned int last_n_entries=0);
  void printTable(std::ostream & out, unsigned int last_n_entries, const MooseEnum & suggested_term_width);
  void printTable(const std::string & file_name);

  /**
   * Method for dumping the table to a csv file - opening and closing the file handle is handled
   */
  void printCSV(const std::string & file_name, int interval=1);

  void printEnsight(const std::string & file_name);
  void writeExodus(ExodusII_IO * ex_out, Real time);
  void makeGnuplot(const std::string & base_file, const std::string & format);

  static MooseEnum getWidthModes();

protected:
  void printTablePiece(std::ostream & out, unsigned int last_n_entries, std::map<std::string, unsigned short> & col_widths,
                       std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end);

  void printOmittedRow(std::ostream & out, std::map<std::string, unsigned short> & col_widths,
                       std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end) const;
  void printRowDivider(std::ostream & out, std::map<std::string, unsigned short> & col_widths,
                       std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end) const;

  void printNoDataRow(char intersect_char, char fill_char,
                      std::ostream & out, std::map<std::string, unsigned short> & col_widths,
                      std::set<std::string>::iterator & col_begin, std::set<std::string>::iterator & col_end) const;


  /**
   * Returns the width of the terminal using sys/ioctl
   */
  unsigned short getTermWidth(bool use_environment) const;

  /**
   * Data structure for the console table
   * The first map creates an association from the independent variable (normally time)
   * to a map of dependent variables and their associated values if they exist
   */
  std::map<Real, std::map<std::string, Real> > _data;

  /// The set of column names updated when data is inserted through the setter methods
  std::set<std::string> _column_names;

  /// The single cell width used for all columns in the table
  static const unsigned short _column_width;

  /// The absolute minimum PPS table width
  static const unsigned short _min_pps_width;

  /// The optional output file stream
  std::ofstream _output_file;
  bool _stream_open;

  /// The last key value inserted
  Real _last_key;
};


#endif //FORMATTEDTABLE_H
