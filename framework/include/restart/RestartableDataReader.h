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
#include "LateRestartableDataRestorer.h"

#include <sstream>
#include <utility>

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

  /**
   * Key that restricts access to restoreLateData to LateRestartableDataRestorer
   */
  class RestoreLateDataKey
  {
    friend class LateRestartableDataRestorer;
    RestoreLateDataKey() {}
    RestoreLateDataKey(const RestoreLateDataKey &) {}
  };

  /**
   * Internal method for restoring late data.
   *
   * To be used by only the LateRestartableDataRestorer.
   */
  void restoreLateData(const std::string & name, const THREAD_ID tid, const RestoreLateDataKey);

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

  /**
   * @return Whether or not data exists in the headers with the name
   * \p data_name with type \p type on thread \p tid
   *
   * Requires that restore() is called first to load the headers.
   */
  bool
  hasData(const std::string & data_name, const std::type_info & type, const THREAD_ID tid) const;

  /**
   * @return Whether or not data with the name \p data_name and type \p type
   * on thread \p tid is available for a late restore
   */
  bool isLateRestorable(const std::string & data_name,
                        const std::type_info & type,
                        const THREAD_ID tid) const;

  /**
   * @return The object used to restore data late
   */
  LateRestartableDataRestorer & getLateRestorer() { return _late_restorer; }

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
    /// Whether or not this data had context
    bool has_context;
  };

  /**
   * Internal method for reading the header (stored by RestartableDataWriter)
   */
  std::vector<std::unordered_map<std::string, HeaderEntry>>
  readHeader(InputStream & header_input) const;

  /**
   * Internal method for deserializing (restoring from backup into a value)
   */
  void deserializeValue(InputStream & data_input,
                        RestartableDataValue & value,
                        const HeaderEntry & header_entry) const;

  /**
   * Checks whether or not we're currently restoring and errors if not
   */
  void requireRestoring() const;

  /**
   * @return The header entry for the data with name \p data_name on thread \p tid
   * if it exists, and nullptr otherwise.
   *
   * Requires that restore() is called first to load the headers.
   */
  const HeaderEntry * queryHeader(const std::string & data_name, const THREAD_ID tid) const;
  /**
   * @return The header entry for the data with name \p data_name on thread \p tid.
   *
   * Requires that restore() is called first to load the headers.
   */
  const HeaderEntry & getHeader(const std::string & data_name, const THREAD_ID tid) const;

  /**
   * @returns Whether or not the type \p type is the same as the type in \p header_entry
   *
   * We need this because this check depends on whether or not we do a string comparison
   */
  bool isSameType(const HeaderEntry & header_entry, const std::type_info & type) const;

  /// The inputs for reading
  InputStreams _streams;

  /// The loaded headers from the restart
  std::vector<std::unordered_map<std::string, HeaderEntry>> _header;

  /// Whether or not we're currently restoring
  bool _is_restoring;

  /// Whether or not to error with a different number of processors
  bool _error_on_different_number_of_processors;

  /// The external API for restoring data late
  LateRestartableDataRestorer _late_restorer;
};
