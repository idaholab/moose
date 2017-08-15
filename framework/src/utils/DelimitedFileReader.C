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

// STL includes
#include <sstream>
#include <iomanip>

// MOOSE includes
#include "DelimitedFileReader.h"
#include "MooseUtils.h"
#include "MooseError.h"

namespace MooseUtils
{

DelimitedFileReader::DelimitedFileReader(const std::string & filename,
                                         const bool header,
                                         const std::string delimiter,
                                         const libMesh::Parallel::Communicator * comm)
  : _filename(filename),
    _header(header),
    _delimiter(delimiter),
    _ignore_empty_lines(true),
    _communicator(comm)
{
}

void
DelimitedFileReader::read()
{
  // Number of columns
  std::size_t n_cols;

  // Storage for the raw data
  std::vector<double> raw;
  std::size_t size_raw;

  // Read data
  if (_communicator == nullptr || _communicator->rank() == 0)
  {
    // Check the file
    MooseUtils::checkFileReadable(_filename);

    // Create the file stream
    std::ifstream stream_data(_filename);

    // Read/generate the header
    initializeColumns(stream_data);

    // Set the number of columns
    n_cols = _column_names.size();

    // Read the data
    readData(stream_data, raw);

    // Close the stream
    stream_data.close();

    // Set raw data vector size
    size_raw = raw.size();
  }

  if (_communicator != nullptr)
  {
    // Broadcast column names
    _communicator->broadcast(n_cols);
    _column_names.resize(n_cols);
    _communicator->broadcast(_column_names);

    // Broadcast raw data
    _communicator->broadcast(size_raw);
    raw.resize(size_raw);
    _communicator->broadcast(raw);
  }

  // Update the data
  _data.resize(n_cols);
  mooseAssert(raw.size() % n_cols == 0,
              "The raw data is not evenly divisible by the number of columns.");
  const std::size_t n_rows = raw.size() / n_cols;
  for (std::size_t j = 0; j < n_cols; ++j)
  {
    _data[j].resize(n_rows);
    for (std::size_t i = 0; i < n_rows; ++i)
      _data[j][i] = raw[i * n_cols + j];
  }
}

const std::vector<std::string> &
DelimitedFileReader::getColumnNames() const
{
  return _column_names;
}

const std::vector<std::vector<double>> &
DelimitedFileReader::getColumnData() const
{
  return _data;
}

const std::vector<double> &
DelimitedFileReader::getColumnData(const std::string & name) const
{
  const auto it = find(_column_names.begin(), _column_names.end(), name);
  if (it == _column_names.end())
    mooseError("Could not find '", name, "' in header of file ", _filename, ".");
  return _data[std::distance(_column_names.begin(), it)];
}

void
DelimitedFileReader::initializeColumns(std::ifstream & stream_data)
{
  // Storage for line content
  std::string line;

  // Read the header (this is the default)
  if (_header)
  {
    std::getline(stream_data, line);
    MooseUtils::tokenize(line, _column_names, 1, _delimiter);
    for (std::string & str : _column_names)
      str = MooseUtils::trim(str);
  }

  // Generate the header
  else
  {
    // Read the first line of data (to set expected column size) and return back to original pos
    std::streampos pos = stream_data.tellg();
    std::getline(stream_data, line);
    MooseUtils::tokenize(line, _column_names, 1, _delimiter);
    stream_data.seekg(pos);

    // Create names
    std::size_t n = _column_names.size();
    int padding = std::log(n);
    for (std::size_t i = 0; i < n; ++i)
    {
      std::stringstream ss;
      ss << "column_" << std::setw(padding) << std::setfill('0') << i;
      _column_names[i] = ss.str();
    }
  }
}

void
DelimitedFileReader::readData(std::ifstream & stream_data, std::vector<double> & output)
{
  // Local storage for the data being read
  std::string line;
  std::vector<double> row;

  // Keep track of the row number for error reporting
  unsigned int count = _header ? 1 : 0;

  // The number of columns expected based on the first row of the data
  const size_t n_cols = _column_names.size();

  // Read the lines
  while (std::getline(stream_data, line))
  {
    // Increment row counter and clear any tokenized data
    count++;
    row.clear();

    // Ignore empty lines
    if (line.empty())
    {
      if (_ignore_empty_lines)
        continue;
      else
        mooseError("Failed to read line ", count, " in file ", _filename, ". The line is empty.");
    }

    // Separate the row and error if it fails
    bool status = MooseUtils::tokenizeAndConvert<double>(line, row, _delimiter);
    if (!status)
      mooseError("Failed to convert a delimited data into double when reading row ",
                 count,
                 " in file ",
                 _filename,
                 ".");

    // Check number of columns
    if (row.size() != n_cols)
      mooseError("The number of columns read (",
                 row.size(),
                 ") does not match the number of columns expected (",
                 n_cols,
                 ") based on the first row of the file when reading row ",
                 count,
                 " in file ",
                 _filename,
                 ".");

    // Append data
    output.insert(output.end(), row.begin(), row.end());
  }
}
}
