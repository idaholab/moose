//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RestartableDataIO.h"

/**
 * Reader for restartable data.
 */
class RestartableDataReader : public RestartableDataIO
{
public:
  RestartableDataReader(MooseApp & app, RestartableDataMap & data);
  RestartableDataReader(MooseApp & app, std::vector<RestartableDataMap> & data);

  /**
   * Sets the input stream for reading to \p stream.
   */
  void setInput(std::shared_ptr<std::istream> stream);

  /**
   * Clears the contents of the reader (stream and cached header)
   */
  void clear();

  /**
   * Restores the restartable data. The input must be set via setInput() first.
   *
   * @param retain Whether or not to retain the data. This is useful for restoring
   * specific pieces of data at a later point after the initial restore.
   * @param filter_names A list of data names to only restore. If not provided,
   * restores all.
   */
  void restore(const bool retain, const DataNames & filter_names = {});
  /**
   * Restores the restartable data via the file at \p file_name.
   *
   * The underlying file will be managed and kept open if \p retain == true
   *
   * @param file_name The filename
   * @param retain Whether or not to retain the data. This is useful for restoring
   * specific pieces of data at a later point after the initial restore.
   * @param filter_names A list of data names to only restore. If not provided,
   * restores all.
   */
  void
  restore(const std::string & file_name, const bool retain, const DataNames & filter_names = {});

  ///@{
  /*
   * Enable/Disable errors to allow meta data to be created/loaded on different number or
   * processors
   *
   * See LoadSurrogateModelAction for use case
   */
  void setErrorOnLoadWithDifferentNumberOfProcessors(bool value)
  {
    _error_on_different_number_of_processors = value;
  }
  ///@}

private:
  /**
   * Struct that describes data in the header
   */
  struct HeaderEntry
  {
    /// The position in the stream at which this data is
    std::streampos position;
    /// The size of this data
    std::size_t size;
    /// The hash code for this data (typeid(T).hash_code())
    std::size_t type_hash_code;
    /// The type for this data
    std::string type;
  };

  /**
   * Internal method for reading the header and storing it into _header
   */
  void readHeader();

  /**
   * Internal methods for deserializing data
   */
  ///@{
  void deserialize(const THREAD_ID tid, const DataNames & filter_names);
  void deserializeValue(RestartableDataValue & value,
                        const RestartableDataReader::HeaderEntry & data_entry);
  ///@}

  /// The stream to read from
  std::shared_ptr<std::istream> _stream;

  /// The loaded headers from the restart
  std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>> _header;

  /// Whether or not to error with a different number of processors
  bool _error_on_different_number_of_processors = true;
};
