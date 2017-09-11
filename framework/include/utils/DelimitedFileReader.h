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

#ifndef DELIMITEDFILEREADER_H
#define DELIMITEDFILEREADER_H

// STL includes
#include <vector>
#include <string>
#include <fstream>

#include "libmesh/parallel.h"

// MOOSE includes
#include "MooseEnum.h"

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
class DelimitedFileReader
{
public:
  enum class HeaderFlag
  {
    FALSE = 0,
    TRUE = 1,
    AUTO = 2
  };

  std::size_t INVALID_SIZE = std::numeric_limits<std::size_t>::max();

  DelimitedFileReader(const std::string & filename,
                      const libMesh::Parallel::Communicator * comm = nullptr);

  /**
   * Perform the actual data reading.
   *
   * This is a separate method to allow for the filename to be read multiple times.
   */
  void read();

  ///@{
  /**
   * Toggles for file format items.
   */
  void setIgnoreEmptyLines(bool value) { _ignore_empty_lines = value; }
  void setFormat(const std::string & value) { _format = value; }
  void setDelimiter(const std::string & value) { _delimiter = value; }
  void setHeader(HeaderFlag value) { _header = value; }
  void setComment(const std::string & value) { _row_comment = value; }
  ///@}

  /**
   * Return the column/row names.
   */
  const std::vector<std::string> & getNames() const;

  /**
   * Return the rows/columns of data.
   *
   * The outer vector is column and the inner the rows.
   */
  const std::vector<std::vector<double>> & getData() const;

  ///@{
  /**
   * Return the row/column of data for a specified header entry
   */
  const std::vector<double> & getData(const std::string & name) const;
  const std::vector<double> & getData(std::size_t index) const;
  ///@}

  ///@{
  /**
   * Deprecated
   */
  void setHeader(bool value);
  const std::vector<std::string> & getColumnNames() const;
  const std::vector<std::vector<double>> & getColumnData() const;
  const std::vector<double> & getColumnData(const std::string & name) const;
  DelimitedFileReader(const std::string & filename,
                      const bool header,
                      const std::string delimiter,
                      const libMesh::Parallel::Communicator * comm = nullptr);
  ///@}

protected:
  /// The supplied filename.
  const std::string _filename;

  /// Flag indicating if the file contains a header.
  HeaderFlag _header;

  /// The delimiter separating the supplied data entires.
  std::string _delimiter;

  /// Flag for ignoring empty lines
  bool _ignore_empty_lines;

  /// Storage for the read or generated column names.
  std::vector<std::string> _names;

  /// Storage for the read data columns.
  std::vector<std::vector<double>> _data;

  /// Communicator
  const libMesh::Parallel::Communicator * _communicator;

  /// Format "rows" vs "columns"
  MooseEnum _format;

  /// Row offsets (only used with _format == "rows")
  std::vector<std::size_t> _row_offsets;

  /// Hide row comments
  std::string _row_comment;

private:
  /**
   * Read or generate header names.
   */
  // unsigned int initializeColumns(std::ifstream & stream_data);

  /**
   * Read the numeric data as columns and return a single vector for broadcasting
   */
  void readColumnData(std::ifstream & stream_data, std::vector<double> & output);

  /**
   * Read the numeric data as rows and return a single vector for broadcasting
   */
  void readRowData(std::ifstream & stream_data, std::vector<double> & output);

  // std::vector<std::string> processLines();//(std::ifstream & stream_data);

  /**
   * Populate supplied vector with content from line.
   * @param line The line to extract data from.
   * @param row The vector to populate.
   * @param num The current line number.
   */
  void processLine(const std::string & line, std::vector<double> & row, const unsigned int & num);

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
   * Determine if header exists.
   */
  bool header(const std::string & line);
};
}

#endif
