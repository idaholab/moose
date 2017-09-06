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

namespace MooseUtils
{

/**
 * Utility class for reading delimited data (e.g., CSV data).
 * @param header (Default: true) When true it is assumed that the first row contains the column
 *                header strings, which are extracted and available in the 'getColumnNames' method.
 *                If false, the names are generated in numeric order: "0", "1", etc.
 * @param delimiter (Default: ",") The delimiter separating the data and header.
 *
 * This class assumes that all data is numeric and can be converted to a C++ double. If a
 * Communicator is provide then it will only read on processor 0 and broadcast the data to all
 * processors. If not provided it will read on all processors.
 */
class DelimitedFileReader
{
public:
  DelimitedFileReader(const std::string & filename,
                      const bool header = true,
                      const std::string delimiter = ",",
                      const libMesh::Parallel::Communicator * comm = nullptr);

  /**
   * Perform the actual data reading.
   *
   * This is a separate method to allow for the filename to be read multiple times.
   */
  void read();

  /**
   * Return the column names.
   */
  const std::vector<std::string> & getColumnNames() const;

  /**
   * Toggle for handling empty lines.
   */
  void setIgnoreEmptyLines(bool value) { _ignore_empty_lines = value; }

  /**
   * Return the columns of data.
   *
   * The outer vector is column and the inner the rows.
   */
  const std::vector<std::vector<double>> & getColumnData() const;

  /**
   * Return the column of data for a specified header entry
   */
  const std::vector<double> & getColumnData(const std::string & name) const;

protected:
  /// The supplied filename.
  const std::string _filename;

  /// Flag indicating if the file contains a header.
  const bool _header;

  /// The delimiter separating the supplied data entires.
  const std::string _delimiter;

  /// Flag for ignoring empty lines
  bool _ignore_empty_lines;

  /// Storage for the read or generated column names.
  std::vector<std::string> _column_names;

  /// Storage for the read data columns.
  std::vector<std::vector<double>> _data;

  // Communicator
  const libMesh::Parallel::Communicator * _communicator;

private:
  /**
   * Read or generate header names.
   */
  void initializeColumns(std::ifstream & stream_data);

  /**
   * Read the numeric data and return a single vector for broadcasting
   */
  void readData(std::ifstream & stream_data, std::vector<double> & output);
};
}

#endif
