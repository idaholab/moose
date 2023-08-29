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

#include "RestartableData.h"
#include "InputStream.h"

#include <sstream>

/**
 * Reader for restartable data written by the RestartableDataWriter.
 */
class RestartableDataReader : public RestartableDataIO
{
public:
  RestartableDataReader(MooseApp & app, RestartableDataMap & data);
  RestartableDataReader(MooseApp & app, std::vector<RestartableDataMap> & data);

  /**
   * Structure that contains the input streams for the reader.
   * One for the header and one for the data.
   */
  struct InputStreams
  {
    std::unique_ptr<InputStream> header;
    std::unique_ptr<InputStream> data;
  };

  /**
   * Sets the input stream for reading from the stringstreams \p header_stream
   * and \p data_stream for the header and data, respectively.
   */
  void setInput(std::unique_ptr<std::stringstream> header_stream,
                std::unique_ptr<std::stringstream> data_stream);
  /**
   * Sets the input stream for reading to the file with the folder base \p folder_base
   */
  void setInput(const std::filesystem::path & folder_base);

  /**
   * @return Whether or not this reader is currently restoring
   */
  bool isRestoring() const { return _streams.data != nullptr; }

  /**
   * Clears the contents of the reader (header stream, data stream, header)
   *
   * This returns ownership of the resulting input in the event that
   * it should be retained
   */
  InputStreams clear();

  /**
   * Restores the restartable data. The input must be set via setInput() first.
   *
   * A handle to the input is still kept after this restore is called! In order to
   * remove that handle, you must call clear()!
   *
   * @param filter_names A list of data names to only restore. If not provided,
   * restores all.
   */
  void restore(const DataNames & filter_names = {});

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

  /**
   * @return Whether or not restartable data is available in the folder \p folder_base
   *
   * Will error if the header is available and the data is not, or if the data is
   * and the header is not.
   */
  static bool isAvailable(const std::filesystem::path & folder_base);

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
   * Internal method for reading the header (stored by RestartableDataWriter)
   */
  std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>>
  readHeader(std::istream & header_stream) const;

  /**
   * Internal methods for deserializing data
   */
  ///@{
  void deserializeValue(std::istream & stream,
                        RestartableDataValue & value,
                        const RestartableDataReader::HeaderEntry & header_entry);
  ///@}

  /// The inputs for reading
  InputStreams _streams;

  /// The loaded headers from the restart
  std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>> _header;

  /// Whether or not to error with a different number of processors
  bool _error_on_different_number_of_processors;
};
