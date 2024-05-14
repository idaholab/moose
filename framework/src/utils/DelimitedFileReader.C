//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// STL includes
#include <sstream>
#include <iomanip>
#include <iterator>

// MOOSE includes
#include "DelimitedFileReader.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "pcrecpp.h"

namespace MooseUtils
{

template <typename T>
DelimitedFileReaderTempl<T>::DelimitedFileReaderTempl(const std::string & filename,
                                                      const libMesh::Parallel::Communicator * comm)
  : _filename(filename),
    _header_flag(HeaderFlag::AUTO),
    _ignore_empty_lines(true),
    _communicator(comm),
    _format_flag(FormatFlag::COLUMNS)
{
}

template <typename T>
void
DelimitedFileReaderTempl<T>::read()
{
  // Number of columns
  std::size_t n_cols;

  // Storage for the raw data
  std::vector<T> raw;
  std::size_t size_raw = 0;
  std::size_t size_offsets = 0;

  // Read data
  if (_communicator == nullptr || _communicator->rank() == 0)
  {
    // Check the file
    MooseUtils::checkFileReadable(_filename);

    // Create the file stream and do nothing if the file is empty
    std::ifstream stream_data(_filename);
    if (stream_data.peek() == std::ifstream::traits_type::eof())
      return;

    // Read/generate the header
    if (_format_flag == FormatFlag::ROWS)
      readRowData(stream_data, raw);
    else
      readColumnData(stream_data, raw);

    // Set the number of columns
    n_cols = _names.size();

    // Close the stream
    stream_data.close();

    // Set raw data vector size
    size_raw = raw.size();
    size_offsets = _row_offsets.size();
  }

  if (_communicator != nullptr)
  {
    // Broadcast column names
    _communicator->broadcast(n_cols);
    _names.resize(n_cols);
    _communicator->broadcast(_names);

    // Broadcast raw data
    _communicator->broadcast(size_raw);
    raw.resize(size_raw);
    _communicator->broadcast(raw);

    // Broadcast row offsets
    if (_format_flag == FormatFlag::ROWS)
    {
      _communicator->broadcast(size_offsets);
      _row_offsets.resize(size_offsets);
      _communicator->broadcast(_row_offsets);
    }
  }

  // Resize the internal storage
  _data.resize(n_cols);

  // Process "row" formatted data
  if (_format_flag == FormatFlag::ROWS)
  {
    typename std::vector<T>::iterator start = raw.begin();
    for (std::size_t j = 0; j < n_cols; ++j)
    {
      _data[j] = std::vector<T>(start, start + _row_offsets[j]);
      std::advance(start, _row_offsets[j]);
    }
  }

  // Process "column" formatted data
  else
  {
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
}

template <typename T>
std::size_t
DelimitedFileReaderTempl<T>::numEntries() const
{
  std::size_t n_entries = 0;
  for (std::size_t i = 0; i < _data.size(); ++i)
    n_entries += _data[i].size();

  return n_entries;
}

template <typename T>
const std::vector<std::string> &
DelimitedFileReaderTempl<T>::getNames() const
{
  return _names;
}

template <typename T>
const std::vector<std::vector<T>> &
DelimitedFileReaderTempl<T>::getData() const
{
  return _data;
}

template <>
const std::vector<Point>
DelimitedFileReaderTempl<double>::getDataAsPoints() const
{
  std::vector<Point> point_data;

  for (std::size_t i = 0; i < _data.size(); ++i)
  {
    Point point;

    // Other checks in this class ensure that each data entry has the same number of values;
    // here we just need to check that each data set has LIBMESH_DIM entries (which we could do by
    // equivalently checking that the total number of entries is divisibly by LIBMESH_DIM
    // _and_ one of these data sets has LIBMESH_DIM entries (consider the fringe case where
    // LIBMESH_DIM is 3, but you accidentally put a point file like
    //   0 0
    //   1 0
    //   2 0
    // where each point is the same length _and_ the total points is still divisible by 3.
    // This check here is more exact.
    if (_data.at(i).size() != LIBMESH_DIM)
      mooseError("Each point in file ", _filename, " must have ", LIBMESH_DIM, " entries");

    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      point(j) = _data.at(i).at(j);

    point_data.push_back(point);
  }

  return point_data;
}

template <typename T>
const std::vector<Point>
DelimitedFileReaderTempl<T>::getDataAsPoints() const
{
  mooseError("Not implemented");
}

template <typename T>
const std::vector<T> &
DelimitedFileReaderTempl<T>::getData(const std::string & name) const
{
  const auto it = find(_names.begin(), _names.end(), name);
  if (it == _names.end())
    mooseError("Could not find '", name, "' in header of file ", _filename, ".");
  return _data[std::distance(_names.begin(), it)];
}

template <typename T>
const std::vector<T> &
DelimitedFileReaderTempl<T>::getData(std::size_t index) const
{
  if (index >= _data.size())
    mooseError("The supplied index ",
               index,
               " is out-of-range for the available data in file '",
               _filename,
               "' which contains ",
               _data.size(),
               " items.");
  return _data[index];
}

template <typename T>
void
DelimitedFileReaderTempl<T>::readColumnData(std::ifstream & stream_data, std::vector<T> & output)
{
  // Local storage for the data being read
  std::string line;
  std::vector<T> row;

  // Keep track of the line number for error reporting
  unsigned int count = 0;

  // Number of columns expected based on the first row of the data
  std::size_t n_cols = INVALID_SIZE;

  // Read the lines
  while (std::getline(stream_data, line))
  {
    // Increment line counter and clear any tokenized data
    count++;
    row.clear();

    // Ignore empty and/or comment lines, if applicable
    if (preprocessLine(line, count))
      continue;

    // Read header, if the header exists and the column names do not exist.
    if (_names.empty() && header(line))
    {
      MooseUtils::tokenize(line, _names, 1, delimiter(line));
      for (std::string & str : _names)
        str = MooseUtils::trim(str);
      continue;
    }

    // Separate the row and error if it fails
    processLine(line, row, count);

    // Set the number of columns
    if (n_cols == INVALID_SIZE)
      n_cols = row.size();

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

  // If the names have not been assigned, create the default names
  if (_names.empty())
  {
    _names.resize(n_cols);
    int padding = MooseUtils::numDigits(n_cols);
    for (std::size_t i = 0; i < n_cols; ++i)
    {
      std::stringstream ss;
      ss << "column_" << std::setw(padding) << std::setfill('0') << i;
      _names[i] = ss.str();
    }
  }
}

template <typename T>
void
DelimitedFileReaderTempl<T>::readRowData(std::ifstream & stream_data, std::vector<T> & output)
{
  // Local storage for the data being read
  std::string line;
  std::vector<T> row;
  unsigned int linenum = 0; // line number in file

  // Clear existing data
  _names.clear();
  _row_offsets.clear();

  // Read the lines
  while (std::getline(stream_data, line))
  {
    // Increment line counter and clear any tokenized data
    linenum++;
    row.clear();

    // Ignore empty lines
    if (preprocessLine(line, linenum))
      continue;

    if (header(line))
    {
      std::size_t index = line.find_first_of(delimiter(line));
      _names.push_back(line.substr(0, index));
      line = line.substr(index);
    }

    // Separate the row and error if it fails
    processLine(line, row, linenum);

    // Store row offsets to allow for un-even rows
    _row_offsets.push_back(row.size());

    // Append data
    output.insert(output.end(), row.begin(), row.end());
  }

  // Assign row names if not provided via header
  if (_names.empty())
  {
    int padding = MooseUtils::numDigits(_row_offsets.size());
    for (std::size_t i = 0; i < _row_offsets.size(); ++i)
    {
      std::stringstream ss;
      ss << "row_" << std::setw(padding) << std::setfill('0') << i;
      _names.push_back(ss.str());
    }
  }
}

template <typename T>
bool
DelimitedFileReaderTempl<T>::preprocessLine(std::string & line, const unsigned int & num)
{
  // Handle row comments
  std::size_t index = _row_comment.empty() ? line.size() : line.find_first_of(_row_comment);
  line = MooseUtils::trim(line.substr(0, index));

  // Ignore empty lines
  if (line.empty())
  {
    if (_ignore_empty_lines)
      return true;
    else
      mooseError("Failed to read line ", num, " in file ", _filename, ". The line is empty.");
  }
  return false;
}

template <typename T>
void
DelimitedFileReaderTempl<T>::processLine(const std::string & line,
                                         std::vector<T> & row,
                                         const unsigned int & num)
{
  // Separate the row and error if it fails
  bool status = MooseUtils::tokenizeAndConvert<T>(line, row, delimiter(line));
  if (!status)
    mooseError("Failed to convert a delimited data into double when reading line ",
               num,
               " in file ",
               _filename,
               ".\n  LINE ",
               num,
               ": ",
               line);
}

template <typename T>
const std::string &
DelimitedFileReaderTempl<T>::delimiter(const std::string & line)
{
  if (_delimiter.empty())
  {
    if (line.find(",") != std::string::npos)
      _delimiter = ",";
    else if (line.find("\t") != std::string::npos)
      _delimiter = "\t";
    else
      _delimiter = " ";
  }
  return _delimiter;
}

template <typename T>
bool
DelimitedFileReaderTempl<T>::header(const std::string & line)
{
  switch (_header_flag)
  {
    case HeaderFlag::OFF:
      return false;
    case HeaderFlag::ON:
      return true;
    default:

      // Attempt to convert the line, if it fails assume it is a header
      std::vector<double> row;
      bool contains_alpha = !MooseUtils::tokenizeAndConvert<double>(line, row, delimiter(line));

      // Based on auto detect set the flag to TRUE|FALSE to short-circuit this check for each line
      // in the case of row data.
      _header_flag = contains_alpha ? HeaderFlag::ON : HeaderFlag::OFF;
      return contains_alpha;
  }
}

template class DelimitedFileReaderTempl<Real>;
template class DelimitedFileReaderTempl<std::string>;
} // MooseUtils
