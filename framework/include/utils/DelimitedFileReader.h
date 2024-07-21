//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// STL includes
#include <vector>
#include <string>
#include <fstream>

#include "libmesh/parallel.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MooseTypes.h"

namespace MooseUtils
{

/**
 * Utility class for reading delimited data (e.g., CSV data).
 * @param filename A string for the filename to read.
 * @param comm A pointer to a Communicator object (see below).
 *
 * This class assumes that all data is numeric and can be converted to a C++ double. If a
 * Communicator is provide then it will only read on processor 0 and broadcast the data to all
 * processors. If not provided it will read on all processors.
 */
template <typename T>
class DelimitedFileReaderTempl
{
public:
  enum class HeaderFlag
  {
    OFF = 0,
    ON = 1,
    AUTO = 2
  };

  enum class FormatFlag
  {
    COLUMNS = 0,
    ROWS = 1
  };

  const std::size_t INVALID_SIZE = std::numeric_limits<std::size_t>::max();

  DelimitedFileReaderTempl(const std::string & filename,
                           const libMesh::Parallel::Communicator * comm = nullptr);

  /**
   * Perform the actual data reading.
   *
   * This is a separate method to allow for the filename to be read multiple times.
   */
  void read();

  /**
   * Get the total number of entries in the file
   * @returns number of entries in file
   */
  std::size_t numEntries() const;

  ///@{
  /**
   * Set/Get methods for file format controls.
   *     IgnoreEmptyLines: When true all empty lines are ignored, when false an error is produced.
   *     FormatFlag: Set the file format (rows vs. columns).
   *     Delimiter: Set the file delimiter (if unset it will be detected).
   *     HeaderFlag: Set the header flag (TRUE used the first row has header, FALSE assumes no
   *                 header, and AUTO will attempt to determine if a header exists).
   *     Comment: Set the comment character, by default no comment character is used.
   */
  void setIgnoreEmptyLines(bool value) { _ignore_empty_lines = value; }
  bool getIgnoreEmptyLines() const { return _ignore_empty_lines; }

  void setFormatFlag(FormatFlag value) { _format_flag = value; }
  FormatFlag getFormatFlag() const { return _format_flag; }

  void setDelimiter(const std::string & value) { _delimiter = value; }
  const std::string & setDelimiter() const { return _delimiter; }

  void setHeaderFlag(HeaderFlag value) { _header_flag = value; }
  HeaderFlag getHeaderFlag() const { return _header_flag; }

  void setComment(const std::string & value) { _row_comment = value; }
  const std::string & getComment() const { return _row_comment; }
  ///@}

  /// Set the file name, used to change the file to read from
  /// We also reset the column/row names as a second read might have different names
  void setFileName(const std::string & new_file)
  {
    _filename = new_file;
    _names.clear();
  }

  /**
   * Return the column/row names.
   */
  const std::vector<std::string> & getNames() const;

  /**
   * Return the rows/columns of data.
   *
   * The outer vector is column and the inner the rows.
   */
  const std::vector<std::vector<T>> & getData() const;

  /**
   * Get the data in Point format. This performs checks that the data
   * is of valid dimensions to do so.
   */
  const std::vector<Point> getDataAsPoints() const;

  ///@{
  /**
   * Return the row/column of data for a specified header entry
   */
  const std::vector<T> & getData(const std::string & name) const;
  const std::vector<T> & getData(std::size_t index) const;
  ///@}

protected:
  /// The supplied filename.
  std::string _filename;

  /// Flag indicating if the file contains a header.
  HeaderFlag _header_flag;

  /// The delimiter separating the supplied data entires.
  std::string _delimiter;

  /// Flag for ignoring empty lines
  bool _ignore_empty_lines;

  /// Storage for the read or generated column names.
  std::vector<std::string> _names;

  /// Storage for the read data columns.
  std::vector<std::vector<T>> _data;

  /// Communicator
  const libMesh::Parallel::Communicator * const _communicator;

  /// Format "rows" vs "columns"
  FormatFlag _format_flag;

  /// Row offsets (only used with _format == "rows")
  std::vector<std::size_t> _row_offsets;

  /// Hide row comments
  std::string _row_comment;

private:
  ///@{
  /**
   * Read the numeric data as rows or columns into a single vector.
   */
  void readColumnData(std::ifstream & stream_data, std::vector<T> & output);
  void readRowData(std::ifstream & stream_data, std::vector<T> & output);
  ///@}

  /**
   * Populate supplied vector with content from line.
   * @param line The line to extract data from.
   * @param row The vector to populate.
   * @param num The current line number.
   */
  void processLine(const std::string & line, std::vector<T> & row, const unsigned int & num);

  /**
   * Check the content of the line and if it should be skipped.
   * @param line Complete line being read.
   * @param num The current line number.
   * @returns True if the line should be skipped.
   */
  bool preprocessLine(std::string & line, const unsigned int & num);

  /**
   * Determine the delimiter.
   *
   * If the setDelimiter method is not called the data is inspected, if a ',' is found it is assumed
   * to be the delimiter as is the case for \t. Otherwise a space is used.
   */
  const std::string & delimiter(const std::string & line);

  /**
   * Return the header flag, if it is set to AUTO attempt to determine if a header exists in line.
   */
  bool header(const std::string & line);
};

typedef DelimitedFileReaderTempl<double> DelimitedFileReader;
typedef DelimitedFileReaderTempl<std::string> DelimitedFileOfStringReader;
}
