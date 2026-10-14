//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DelimitedFileReader.h"

namespace MooseUtils
{

/**
 * Utility class for reading delimited data (e.g., CSV data).
 * where the csv has columns of mixed data types
 * @param filename A string for the filename to read.
 *
 * This class allows csv files to contain columns of mixed data types.
 * All of the data is internally stored as string data and then converted to the requested type when
 * getData<T> is called.
 * If a communicator is provide then it will only read on processor 0 and broadcast the data to all
 * processors. If not provided it will read on all processors.
 */
class MixedDataDelimitedFileReader : public DelimitedFileOfStringReader
{
public:
  MixedDataDelimitedFileReader(const std::string & filename) : DelimitedFileOfStringReader(filename)
  {
  }

  MixedDataDelimitedFileReader(const std::string & filename,
                               const libMesh::Parallel::Communicator * comm)
    : DelimitedFileOfStringReader(filename, comm)
  {
  }

  /**
   * Converts a specific named column in the data set to a vector of a specified type
   *
   * @tparam The type to convert the column to
   * @param name the name of the column in the data file
   *
   * @returns the named column as a vector of the requested type
   *
   * @throws mooseError if the requested column name does not exist within the dataset
   * @throws std::invalid_argument if one of the entries in the requested column cannot be converted
   * to the requested type
   */
  template <typename T>
  const std::vector<T> getData(const std::string & name) const
  {
    return convertVector<T>(DelimitedFileOfStringReader::getData(name));
  }

  /**
   * Converts a column in the data set to a vector of a specified type
   *
   * @tparam T The type to convert the column to
   * @param index_column the index of the desired column
   *
   * @returns The requested column as a vector of the requested type
   *
   * @throws mooseError if the requested column index does not exist within the dataset
   * @throws std::invalid_argument if one of the entries in the requested column cannot be converted
   * to the requested type
   */
  template <typename T>
  const std::vector<T> getData(std::size_t index_column) const
  {
    return convertVector<T>(DelimitedFileOfStringReader::getData(index_column));
  }

protected:
  /**
   * Given a column of a data set this will convert it to a vector of the requested type
   *
   * @tparam T the type to convert the column to
   * @param column the column of the dataset provided as a string
   *
   * @returns The provided vector of strings converted to a vector of the requested type
   *
   * @throws mooseError if the requested column index does not exist within the dataset
   * @throws std::invalid_argument if one of the entries in the requested column cannot be converted
   * to the requested type
   */
  template <typename T>
  std::vector<T> convertVector(const std::vector<std::string> & column) const
  {
    auto converted_data = std::vector<T>(column.size());
    for (size_t index_row = 0; index_row < column.size(); ++index_row)
    {
      convert<T>(column[index_row], converted_data[index_row], true);
    }
    return converted_data;
  }
};
}
