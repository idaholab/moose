//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseEnum.h"
#include "DataIO.h"
#include "MooseUtils.h"

// C++ includes
#include <fstream>

// Forward declarations
class FormattedTable;
class TableValueBase;
namespace libMesh
{
class ExodusII_IO;
}

template <>
void dataStore(std::ostream & stream, FormattedTable & table, void * context);
template <>
void dataLoad(std::istream & stream, FormattedTable & v, void * context);
template <>
void dataStore(std::ostream & stream, TableValueBase *& value, void * context);
template <>
void dataLoad(std::istream & stream, TableValueBase *& value, void * context);

class TableValueBase
{
public:
  virtual ~TableValueBase() = default;

  template <typename T>
  static constexpr bool isSupportedType()
  {
    return std::is_fundamental<T>::value || std::is_same<T, std::string>::value;
  }

  virtual void print(std::ostream & os) const = 0;

  virtual void store(std::ostream & stream, void * context) = 0;
};

std::ostream & operator<<(std::ostream & os, const TableValueBase & value);

template <typename T>
class TableValue : public TableValueBase
{
public:
  TableValue(const T & value) : _value(value)
  {
    if (!this->isSupportedType<T>())
      mooseError("Unsupported type ", MooseUtils::prettyCppType<T>(), " for FormattedTable.");
  }

  const T & get() const { return _value; }
  T & set() { return _value; }

  virtual void print(std::ostream & os) const override { os << this->_value; };

  virtual void store(std::ostream & stream, void * context) override;
  static void
  load(std::istream & stream, std::shared_ptr<TableValueBase> & value_base, void * context);

private:
  T _value;
};

template <>
inline void
TableValue<bool>::print(std::ostream & os) const
{
  os << (this->_value ? "True" : "False");
}

template <typename T>
void
TableValue<T>::store(std::ostream & stream, void * context)
{
  std::string type = typeid(T).name();
  ::dataStore(stream, type, context);
  ::dataStore(stream, _value, context);
}

template <typename T>
void
TableValue<T>::load(std::istream & stream,
                    std::shared_ptr<TableValueBase> & value_base,
                    void * context)
{
  T value;
  ::dataLoad(stream, value, context);
  value_base = std::dynamic_pointer_cast<TableValueBase>(std::make_shared<TableValue<T>>(value));
}

/**
 * This class is used for building, formatting, and outputting tables of numbers.
 */
class FormattedTable
{
public:
  /**
   * Default constructor - The default constructor takes an optional parameter to turn off
   * stateful printing. This means that each time you ask the FormattedTable to print to a file,
   * it'll, print the entire table. The default is to only print the part of the table that hasn't
   * already been printed.
   */
  FormattedTable();

  /**
   * Copy constructor - The copy constructor will duplicate the data structures but is not
   * designed to work with FormattedTables with open streams (e.g. CSV Output mode).
   */
  FormattedTable(const FormattedTable & o);

  /**
   * The destructor is used to close the file handle
   */
  ~FormattedTable();

  /**
   * Returns a boolean value based on whether the FormattedTable contains data or not
   */
  bool empty() const;

  /**
   * Sets append mode which means an existing file is not truncated on opening. This mode
   * is typically used for recovery.
   */
  void append(bool append_existing_file);

  /**
   * Force a new row in the table with the passed in time.
   */
  void addRow(Real time);

  /**
   * Method for adding data to the output table. Data is added to the last row. Method will
   * error if called on an empty table.
   */
  template <typename T = Real>
  void addData(const std::string & name, const T & value);

  /**
   * Method for adding data to the output table.  The dependent variable is named "time"
   */
  template <typename T = Real>
  void addData(const std::string & name, const T & value, Real time);

  /**
   * Method for adding an entire vector to a table at a time. Checks are made to ensure that
   * the dependent variable index lines up with the vector indices.
   */
  template <typename T = Real>
  void addData(const std::string & name, const std::vector<T> & vector);

  /**
   * Retrieve the last time (or independent variable) value.
   */
  Real getLastTime();

  /**
   * Retrieve Data for last value of given name
   */
  template <typename T = Real>
  T & getLastData(const std::string & name);

  void clear();

  /**
   * Set whether or not to output time column.
   */
  void outputTimeColumn(bool output_time) { _output_time = output_time; }

  //  const std::map<Real, std::map<std::string, Real>> & getData() const { return _data; }

  /**
   * Methods for dumping the table to the stream - either by filename or by stream handle.  If
   * a filename is supplied opening and closing of the file is properly handled.  In the
   * screen version of the method, an optional parameters can be passed to print only the last
   * "n" entries.  A value of zero means don't skip any rows
   *
   * Note: Only call these from processor 0!
   */
  void printTable(std::ostream & out, unsigned int last_n_entries = 0);
  void printTable(std::ostream & out,
                  unsigned int last_n_entries,
                  const MooseEnum & suggested_term_width);
  void printTable(const std::string & file_name);

  /**
   * Method for dumping the table to a csv file - opening and closing the file handle is handled
   *
   * Note: Only call this on processor 0!
   */
  void printCSV(const std::string & file_name, int interval = 1, bool align = false);

  void printEnsight(const std::string & file_name);
  void writeExodus(ExodusII_IO * ex_out, Real time);
  void makeGnuplot(const std::string & base_file, const std::string & format);

  static MooseEnum getWidthModes();

  /**
   * By default printCSV places "," between each entry, this allows this to be changed
   */
  void setDelimiter(std::string delimiter) { _csv_delimiter = delimiter; }

  /**
   * By default printCSV prints output to a precision of 14, this allows this to be changed
   */
  void setPrecision(unsigned int precision) { _csv_precision = precision; }

  /**
   * Sorts columns alphabetically.
   */
  void sortColumns();

protected:
  void printTablePiece(std::ostream & out,
                       unsigned int last_n_entries,
                       std::map<std::string, unsigned short> & col_widths,
                       std::vector<std::string>::iterator & col_begin,
                       std::vector<std::string>::iterator & col_end);

  void printOmittedRow(std::ostream & out,
                       std::map<std::string, unsigned short> & col_widths,
                       std::vector<std::string>::iterator & col_begin,
                       std::vector<std::string>::iterator & col_end) const;
  void printRowDivider(std::ostream & out,
                       std::map<std::string, unsigned short> & col_widths,
                       std::vector<std::string>::iterator & col_begin,
                       std::vector<std::string>::iterator & col_end) const;

  void printNoDataRow(char intersect_char,
                      char fill_char,
                      std::ostream & out,
                      std::map<std::string, unsigned short> & col_widths,
                      std::vector<std::string>::iterator & col_begin,
                      std::vector<std::string>::iterator & col_end) const;

  /**
   * Returns the width of the terminal using sys/ioctl
   */
  unsigned short getTermWidth(bool use_environment) const;

  /**
   * Data structure for the console table:
   * The first part of the pair tracks the independent variable (normally time) and is associated
   * with the second part of the table which is the map of dependent variables and their associated
   * values.
   */
  std::vector<std::pair<Real, std::map<std::string, std::shared_ptr<TableValueBase>>>> _data;

  /// Alignment widths (only used if asked to print aligned to CSV output)
  std::map<std::string, unsigned int> _align_widths;

  /// The set of column names updated when data is inserted through the setter methods
  std::vector<std::string> _column_names;

  /// The single cell width used for all columns in the table
  static const unsigned short _column_width;

  /// The absolute minimum PPS table width
  static const unsigned short _min_pps_width;

private:
  /// Close the underlying output file stream if any. This is idempotent.
  void close();

  /// Open or switch the underlying file stream to point to file_name. This is idempotent.
  void open(const std::string & file_name);

  void printRow(std::pair<Real, std::map<std::string, std::shared_ptr<TableValueBase>>> & row_data,
                bool align);

  /// Fill any values that are not defined (usually when there are mismatched column lengths)
  void fillEmptyValues();

  /// The optional output file stream
  std::string _output_file_name;

  /// The stream handle (corresponds to _output_file_name)
  std::ofstream _output_file;

  /**
   * Keeps track of the index indicating which vector elements have been output. All items
   * with an index less than this index have been output. Higher values have not.
   */
  std::size_t _output_row_index;

  /**
   * Keeps track of whether the header has been output. This is separate from _output_row_index
   * because it's possible to output the header with zero rows. We don't consider this a bug,
   * it helps users understand that they have declared vectors properly but maybe haven't populated
   * them correctly.
   */
  bool _headers_output;

  /// Keeps track of whether we want to open an existing file for appending or overwriting.
  bool _append;

  /// Whether or not to output the Time column
  bool _output_time;

  /// *.csv file delimiter, defaults to ","
  std::string _csv_delimiter;

  /// *.csv file precision, defaults to 14
  unsigned int _csv_precision;

  /// Flag indicating that sorting is necessary (used by sortColumns method).
  bool _column_names_unsorted = true;

  friend void
  dataStore<FormattedTable>(std::ostream & stream, FormattedTable & table, void * context);
  friend void dataLoad<FormattedTable>(std::istream & stream, FormattedTable & v, void * context);
};

template <typename T>
void
FormattedTable::addData(const std::string & name, const T & value)
{
  if (empty())
    mooseError("No Data stored in the the FormattedTable");

  auto back_it = _data.rbegin();
  back_it->second[name] =
      std::dynamic_pointer_cast<TableValueBase>(std::make_shared<TableValue<T>>(value));

  if (std::find(_column_names.begin(), _column_names.end(), name) == _column_names.end())
  {
    _column_names.push_back(name);
    _column_names_unsorted = true;
  }
}

template <typename T>
void
FormattedTable::addData(const std::string & name, const T & value, Real time)
{
  auto back_it = _data.rbegin();

  mooseAssert(back_it == _data.rend() || !MooseUtils::absoluteFuzzyLessThan(time, back_it->first),
              "Attempting to add data to FormattedTable with the dependent variable in a "
              "non-increasing order.\nDid you mean to use addData(std::string &, const "
              "std::vector<Real> &)?");

  // See if the current "row" is already in the table
  if (back_it == _data.rend() || !MooseUtils::absoluteFuzzyEqual(time, back_it->first))
  {
    _data.emplace_back(time, std::map<std::string, std::shared_ptr<TableValueBase>>());
    back_it = _data.rbegin();
  }
  // Insert or update value
  back_it->second[name] =
      std::dynamic_pointer_cast<TableValueBase>(std::make_shared<TableValue<T>>(value));

  if (std::find(_column_names.begin(), _column_names.end(), name) == _column_names.end())
  {
    _column_names.push_back(name);
    _column_names_unsorted = true;
  }
}

template <typename T>
void
FormattedTable::addData(const std::string & name, const std::vector<T> & vector)
{
  for (MooseIndex(vector) i = 0; i < vector.size(); ++i)
  {
    if (i == _data.size())
      _data.emplace_back(i, std::map<std::string, std::shared_ptr<TableValueBase>>());

    mooseAssert(MooseUtils::absoluteFuzzyEqual(_data[i].first, i),
                "Inconsistent indexing in VPP vector");

    auto & curr_entry = _data[i];
    curr_entry.second[name] =
        std::dynamic_pointer_cast<TableValueBase>(std::make_shared<TableValue<T>>(vector[i]));
  }

  if (std::find(_column_names.begin(), _column_names.end(), name) == _column_names.end())
  {
    _column_names.push_back(name);
    _column_names_unsorted = true;
  }
}

template <typename T>
T &
FormattedTable::getLastData(const std::string & name)
{
  mooseAssert(!empty(), "No Data stored in the FormattedTable");

  auto & last_data_map = _data.rbegin()->second;
  auto it = last_data_map.find(name);
  if (it == last_data_map.end())
    mooseError("No Data found for name: " + name);

  auto value = std::dynamic_pointer_cast<TableValue<T>>(it->second);
  if (!value)
    mooseError("Data for ", name, " is not of the requested type.");
  return value->set();
}

template <>
void dataStore(std::ostream & stream, FormattedTable & table, void * context);
template <>
void dataLoad(std::istream & stream, FormattedTable & v, void * context);
template <>
void dataStore(std::ostream & stream, TableValueBase *& value, void * context);
template <>
void dataLoad(std::istream & stream, TableValueBase *& value, void * context);
