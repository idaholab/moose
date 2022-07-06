//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
/**
 * Note!  This class comes from https://github.com/friedmud/variadic_table
 *
 *
 * DO NOT MODIFY THIS CLASS HERE... MODIFY IT THERE AND COPY IT HERE
 *
 */

#include <iostream>
#include <iomanip>
#include <ios>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cassert>
#include <cmath>

/**
 * Used to specify the column format
 */
enum class VariadicTableColumnFormat
{
  AUTO,
  SCIENTIFIC,
  FIXED,
  PERCENT
};

/**
 * A class for "pretty printing" a table of data.
 *
 * Requries C++11 (and nothing more)
 *
 * It's templated on the types that will be in each column
 * (all values in a column must have the same type)
 *
 * For instance, to use it with data that looks like:  "Fred", 193.4, 35, "Sam"
 * with header names: "Name", "Weight", "Age", "Brother"
 *
 * You would invoke the table like so:
 * VariadicTable<std::string, double, int, std::string> vt("Name", "Weight", "Age", "Brother");
 *
 * Then add the data to the table:
 * vt.addRow("Fred", 193.4, 35, "Sam");
 *
 * And finally print it:
 * vt.print();
 */
template <class... Ts>
class VariadicTable
{
public:
  /// The type stored for each row
  typedef std::tuple<Ts...> DataTuple;

  /**
   * Construct the table with headers
   *
   * @param headers The names of the columns
   * @param static_column_size The size of columns that can't be found automatically
   */
  VariadicTable(std::vector<std::string> headers,
                unsigned int static_column_size = 0,
                unsigned int cell_padding = 1)
    : _headers(headers),
      _num_columns(std::tuple_size<DataTuple>::value),
      _static_column_size(static_column_size),
      _cell_padding(cell_padding)
  {
    assert(headers.size() == _num_columns);
  }

  /**
   * Add a row of data
   *
   * Easiest to use like:
   * table.addRow({data1, data2, data3});
   *
   * @param data A Tuple of data to add
   */
  void addRow(Ts... entries) { _data.emplace_back(std::make_tuple(entries...)); }

  /**
   * Pretty print the table of data
   */
  template <typename StreamType>
  void print(StreamType & stream)
  {
    size_columns();

    // Start computing the total width
    // First - we will have _num_columns + 1 "|" characters
    unsigned int total_width = _num_columns + 1;

    // Now add in the size of each colum
    for (auto & col_size : _column_sizes)
      total_width += col_size + (2 * _cell_padding);

    // Print out the top line
    stream << std::string(total_width, '-') << "\n";

    // Print out the headers
    stream << "|";
    for (unsigned int i = 0; i < _num_columns; i++)
    {
      // Must find the center of the column
      auto half = _column_sizes[i] / 2;
      half -= _headers[i].size() / 2;

      stream << std::string(_cell_padding, ' ') << std::setw(_column_sizes[i]) << std::left
             << std::string(half, ' ') + _headers[i] << std::string(_cell_padding, ' ') << "|";
    }

    stream << "\n";

    // Print out the line below the header
    stream << std::string(total_width, '-') << "\n";

    // Now print the rows of the table
    for (auto & row : _data)
    {
      stream << "|";
      print_each(row, stream);
      stream << "\n";
    }

    // Print out the line below the header
    stream << std::string(total_width, '-') << std::endl;
  }

  /**
   * Set how to format numbers for each column
   *
   * Note: this is ignored for std::string columns
   *
   * @column_format The format for each column: MUST be the same length as the number of columns.
   */
  void setColumnFormat(const std::vector<VariadicTableColumnFormat> & column_format)
  {
    assert(column_format.size() == std::tuple_size<DataTuple>::value);

    _column_format = column_format;
  }

  /**
   * Set how many digits of precision to show for floating point numbers
   *
   * Note: this is ignored for std::string columns
   *
   * @column_format The precision for each column: MUST be the same length as the number of columns.
   */
  void setColumnPrecision(const std::vector<int> & precision)
  {
    assert(precision.size() == std::tuple_size<DataTuple>::value);
    _precision = precision;
  }

protected:
  // Attempts to figure out the correct justification for the data
  template <typename T>
  static auto justify(int /*firstchoice*/)
  {
    if constexpr (std::is_arithmetic<typename std::remove_reference<T>::type>::value)
      // If it's a floating point value
      return std::right;
    else
      // Otherwise
      return std::left;
  }

  /**
   * These three functions print out each item in a Tuple into the table
   *
   * Original Idea From From https://stackoverflow.com/a/26908596
   *
   * BTW: This would all be a lot easier with generic lambdas
   * there would only need to be one of this sequence and then
   * you could pass in a generic lambda.  Unfortunately, that's C++14
   */

  /**
   * This gets called on each item
   */
  template <std::size_t I, typename TupleType, typename StreamType>
  void print_each(TupleType && t, StreamType & stream, std::integral_constant<size_t, I>)
  {
    auto & val = std::get<I>(t);

    auto original_precision = stream.precision();
    auto original_flags = stream.flags();

    // Set the precision
    if (!_precision.empty())
    {
      assert(_precision.size() ==
             std::tuple_size<typename std::remove_reference<TupleType>::type>::value);

      stream << std::setprecision(_precision[I]);
    }

    // Set the format
    if (!_column_format.empty())
    {
      assert(_column_format.size() ==
             std::tuple_size<typename std::remove_reference<TupleType>::type>::value);

      if (_column_format[I] == VariadicTableColumnFormat::SCIENTIFIC)
        stream << std::scientific;

      if (_column_format[I] == VariadicTableColumnFormat::FIXED)
        stream << std::fixed;

      if (_column_format[I] == VariadicTableColumnFormat::PERCENT)
        stream << std::fixed << std::setprecision(2);
    }

    stream << std::string(_cell_padding, ' ') << std::setw(_column_sizes[I])
           << justify<decltype(val)>(0) << val << std::string(_cell_padding, ' ') << "|";

    // Restore the format
    if (!_column_format.empty())
      stream.flags(original_flags);

    // Restore the precision
    if (!_precision.empty())
      stream.precision(original_precision);

    // Recursive call to print the next item (if there are any left)
    if constexpr (I + 1 != std::tuple_size<typename std::remove_reference<TupleType>::type>::value)
      print_each(std::forward<TupleType>(t), stream, std::integral_constant<size_t, I + 1>());
  }

  /**
   * This is what gets called first
   */
  template <typename TupleType, typename StreamType>
  void print_each(TupleType && t, StreamType & stream)
  {
    print_each(std::forward<TupleType>(t), stream, std::integral_constant<size_t, 0>());
  }

  /**
   * Try to find the size the column will take up
   *
   * If the datatype has a size() member... let's call it
   */
  template <class T>
  size_t sizeOfData(const T & data, decltype(((T *)nullptr)->size()) * /*dummy*/ = nullptr)
  {
    return data.size();
  }

  /**
   * Try to find the size the column will take up
   *
   * If the datatype is an integer - let's get it's length
   */
  template <class T>
  size_t sizeOfData(const T & data,
                    typename std::enable_if<std::is_integral<T>::value>::type * /*dummy*/ = nullptr)
  {
    if (data == 0)
      return 1;

    return std::log10(data) + 1;
  }

  /**
   * If it doesn't... let's just use a statically set size
   */
  size_t sizeOfData(...) { return _static_column_size; }

  /**
   * These three functions iterate over the Tuple, find the printed size of each element and set it
   * in a vector
   */

  /**
   * End the recursion
   */
  template <typename TupleType>
  void size_each(TupleType &&,
                 std::vector<unsigned int> & /*sizes*/,
                 std::integral_constant<
                     size_t,
                     std::tuple_size<typename std::remove_reference<TupleType>::type>::value>)
  {
  }

  /**
   * Recursively called for each element
   */
  template <std::size_t I,
            typename TupleType,
            typename = typename std::enable_if<
                I != std::tuple_size<typename std::remove_reference<TupleType>::type>::value>::type>
  void
  size_each(TupleType && t, std::vector<unsigned int> & sizes, std::integral_constant<size_t, I>)
  {
    sizes[I] = sizeOfData(std::get<I>(t));

    // Override for Percent
    if (!_column_format.empty())
      if (_column_format[I] == VariadicTableColumnFormat::PERCENT)
        sizes[I] = 6; // 100.00

    // Continue the recursion
    size_each(std::forward<TupleType>(t), sizes, std::integral_constant<size_t, I + 1>());
  }

  /**
   * The function that is actually called that starts the recursion
   */
  template <typename TupleType>
  void size_each(TupleType && t, std::vector<unsigned int> & sizes)
  {
    size_each(std::forward<TupleType>(t), sizes, std::integral_constant<size_t, 0>());
  }

  /**
   * Finds the size each column should be and set it in _column_sizes
   */
  void size_columns()
  {
    _column_sizes.resize(_num_columns);

    // Temporary for querying each row
    std::vector<unsigned int> column_sizes(_num_columns);

    // Start with the size of the headers
    for (unsigned int i = 0; i < _num_columns; i++)
      _column_sizes[i] = _headers[i].size();

    // Grab the size of each entry of each row and see if it's bigger
    for (auto & row : _data)
    {
      size_each(row, column_sizes);

      for (unsigned int i = 0; i < _num_columns; i++)
        _column_sizes[i] = std::max(_column_sizes[i], column_sizes[i]);
    }
  }

  /// The column headers
  std::vector<std::string> _headers;

  /// Number of columns in the table
  unsigned int _num_columns;

  /// Size of columns that we can't get the size of
  unsigned int _static_column_size;

  /// Size of the cell padding
  unsigned int _cell_padding;

  /// The actual data
  std::vector<DataTuple> _data;

  /// Holds the printable width of each column
  std::vector<unsigned int> _column_sizes;

  /// Column Format
  std::vector<VariadicTableColumnFormat> _column_format;

  /// Precision For each column
  std::vector<int> _precision;
};
