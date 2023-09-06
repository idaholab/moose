//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataReader.h"

#include "StringInputStream.h"
#include "FileInputStream.h"
#include "RestartableDataMap.h"
#include "MooseUtils.h"

#include <fstream>

// Type hash codes can't be relied on for older clang...
// not sure why, and also don't care why
#if defined(__clang__) && __clang_major__ < 12
#define RESTARTABLE_SKIP_CHECK_HASH_CODE
#endif

RestartableDataReader::RestartableDataReader(MooseApp & app, RestartableDataMap & data)
  : RestartableDataIO(app, data),
    _is_restoring(false),
    _error_on_different_number_of_processors(true)
{
}

RestartableDataReader::RestartableDataReader(MooseApp & app, std::vector<RestartableDataMap> & data)
  : RestartableDataIO(app, data),
    _is_restoring(false),
    _error_on_different_number_of_processors(true)
{
}

void
RestartableDataReader::setInput(std::unique_ptr<std::stringstream> header_stream,
                                std::unique_ptr<std::stringstream> data_stream)
{
  mooseAssert(!_streams.header, "Header input already set");
  mooseAssert(!_streams.data, "Data stream already set");
  _streams.header = std::make_unique<StringInputStream>(std::move(header_stream));
  _streams.data = std::make_unique<StringInputStream>(std::move(data_stream));
}

void
RestartableDataReader::setInput(const std::filesystem::path & folder_base)
{
  mooseAssert(!_streams.header, "Header input already set");
  mooseAssert(!_streams.data, "Data stream already set");
  _streams.header = std::make_unique<FileInputStream>(restartableHeaderFile(folder_base));
  _streams.data = std::make_unique<FileInputStream>(restartableDataFile(folder_base));
}

RestartableDataReader::InputStreams
RestartableDataReader::clear()
{
  _is_restoring = false;
  _header.clear();
  InputStreams streams;
  std::swap(_streams, streams);
  return streams;
}

std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>>
RestartableDataReader::readHeader(InputStream & header_input) const
{
  std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>> header;

  auto stream_ptr = header_input.get();
  auto & stream = *stream_ptr;

  stream.seekg(0);

  const auto error = [&header_input](auto... args)
  {
    std::stringstream err_prefix;
    err_prefix << "While reading restartable data in ";
    const auto filename = header_input.getFilename();
    if (filename)
      err_prefix << std::filesystem::absolute(filename->parent_path());
    else
      err_prefix << "memory";
    err_prefix << ":\n\n";

    mooseError(err_prefix.str(), args...);
  };

  // ID
  char this_id[2];
  stream.read(this_id, 2);
  if (this_id[0] != 'R' || this_id[1] != 'D')
    error("The data is invalid or corrupted (unexpected header)");

  // File version
  std::remove_const<decltype(CURRENT_BACKUP_FILE_VERSION)>::type this_file_version;
  dataLoad(stream, this_file_version, nullptr);
  if (this_file_version != CURRENT_BACKUP_FILE_VERSION)
    error("There is a mismatch in the backup version\n\n",
          "Current backup version: ",
          CURRENT_BACKUP_FILE_VERSION,
          "\nLoaded backup version: ",
          this_file_version);

  // Type id for a basic type
  std::size_t this_compare_hash_code;
  dataLoad(stream, this_compare_hash_code, nullptr);
#ifndef RESTARTABLE_SKIP_CHECK_HASH_CODE
  if (this_compare_hash_code != typeid(COMPARE_HASH_CODE_TYPE).hash_code())
    error("The backup is not compatible\n\nThe hash code check for a basic type (",
          MooseUtils::prettyCppType<COMPARE_HASH_CODE_TYPE>(),
          ") failed.\nIt is likely that this backup was stored with a different architecture or "
          "operating system");
#endif

  // Number of procs
  decltype(n_processors()) this_n_procs = 0;
  dataLoad(stream, this_n_procs, nullptr);
  if (_error_on_different_number_of_processors && this_n_procs != n_processors())
    error("The number of MPI ranks is not consistent\n\nCurrent MPI ranks: ",
          n_processors(),
          "\nLoaded MPI ranks: ",
          this_n_procs);

  // Number of data
  decltype(dataSize()) this_num_data = 0;
  dataLoad(stream, this_num_data, nullptr);
  if (this_num_data != dataSize())
    error("The number of threads is not consistent\n\nCurrent threads: ",
          dataSize(),
          "\nLoaded threads: ",
          this_num_data);

  header.resize(dataSize());

  // Size of data for each thread
  std::vector<std::size_t> tid_n_data(dataSize());
  for (const auto tid : make_range(dataSize()))
    dataLoad(stream, tid_n_data[tid], nullptr);

  // The position of the current data that we're loading
  std::size_t current_data_position = 0;

  // Load the data header for each thread
  for (const auto tid : make_range(dataSize()))
    for (const auto i : make_range(tid_n_data[tid]))
    {
      std::ignore = i;

      std::string name;
      dataLoad(stream, name, nullptr);
      mooseAssert(name.size(), "Empty name");

      mooseAssert(!header[tid].count(name), "Data '" + name + "' is already inserted");
      auto & entry = header[tid][name];

      dataLoad(stream, entry.size, nullptr);
      dataLoad(stream, entry.type_hash_code, nullptr);
      dataLoad(stream, entry.type, nullptr);
      dataLoad(stream, entry.has_context, nullptr);
      entry.position = current_data_position;

      current_data_position += entry.size;
    }

  stream.seekg(0);

  return header;
}

void
RestartableDataReader::restore(const DataNames & filter_names /* = {} */)
{
  if (!_streams.header || !_streams.data)
    mooseError("RestartableDataReader::restore(): Cannot restore because an input was not set");

  _is_restoring = true;

  // Set everything as not loaded
  for (const auto tid : make_range(dataSize()))
    for (auto & value : currentData(tid))
      value.setNotLoaded({});

  // Read the header
  _header = readHeader(*_streams.header);

  for (const auto tid : index_range(_header))
  {
    auto & data = currentData(tid);
    const auto & header = _header[tid];

    // TODO: Think about what to do with missing data
    // Load the data in the order that it was requested
    for (auto & value : data)
    {
      const auto & name = value.name();

      auto find_header = header.find(name);
      if (find_header == header.end())
        continue;

      auto & header_entry = find_header->second;

      // Only restore values if we're either recovering or the data isn't filtered out
      const auto is_data_in_filter = filter_names.find(name) != filter_names.end();
      if (!is_data_in_filter)
        deserializeValue(*_streams.data, value, header_entry);
    }
  }
}

bool
RestartableDataReader::hasData(const std::string & data_name,
                               const std::type_info & type,
                               const THREAD_ID tid) const
{
  if (const auto header = queryHeader(data_name, tid))
    return isSameType(*header, type);
  return false;
}

const RestartableDataReader::HeaderEntry *
RestartableDataReader::queryHeader(const std::string & data_name, const THREAD_ID tid) const
{
  requireRestoring();
  const auto it = _header[tid].find(data_name);
  if (it == _header[tid].end())
    return nullptr;
  return &it->second;
}

const RestartableDataReader::HeaderEntry &
RestartableDataReader::getHeader(const std::string & data_name, const THREAD_ID tid) const
{
  const auto header = queryHeader(data_name, tid);
  if (!header)
    mooseError(
        "RestartableDataReader::getHeader(): Failed to find a header entry for data with name '",
        data_name,
        "'");
  return *header;
}

void
RestartableDataReader::deserializeValue(
    InputStream & data_input,
    RestartableDataValue & value,
    const RestartableDataReader::HeaderEntry & header_entry) const
{
  mooseAssert(!value.loaded(), value.name() + " is already loaded");

  auto error = [&data_input, &value](auto... args)
  {
    std::stringstream err;
    err << "While loading restartable data\n\n";
    err << "From: ";
    const auto filename = data_input.getFilename();
    if (filename)
      err << std::filesystem::absolute(filename->parent_path());
    else
      err << "memory";
    err << "\nData name: " << value.name();
    err << "\nData type: " << value.type();
    err << "\n\n";
    mooseError(err.str(), args...);
  };

  if (!isSameType(header_entry, value.typeId()))
    error("The stored type of '", header_entry.type, "' does not match");

  auto stream_ptr = data_input.get();
  auto & stream = *stream_ptr;

  stream.seekg(header_entry.position);
  value.load(stream);

  if (stream.tellg() == -1)
    error("An error was encountered when reading from the stream");

  const std::size_t loaded_size = stream.tellg() - header_entry.position;
  if (loaded_size != header_entry.size)
    error("The data read does not match the data stored\n\n",
          "Stored size: ",
          header_entry.size,
          "\nLoaded size: ",
          loaded_size);
}

bool
RestartableDataReader::isAvailable(const std::filesystem::path & folder_base)
{
  const auto header_path = restartableDataFile(folder_base);
  const auto data_path = restartableDataFile(folder_base);

  const auto available = [](const auto & filename)
  { return MooseUtils::pathExists(filename) && MooseUtils::checkFileReadable(filename); };

  const bool header_available = available(header_path);
  const bool data_available = available(data_path);

  if (header_available != data_available)
    mooseError("The restart ",
               header_available ? "header" : "data",
               " is available but the corresponding ",
               header_available ? "data" : "header",
               " is not available\n\n",
               "Header (",
               header_available ? "available" : "missing",
               "): ",
               std::filesystem::absolute(header_path),
               "\nData (",
               data_available ? "available" : "missing",
               "): ",
               std::filesystem::absolute(header_path));

  return header_available && data_available;
}

void
RestartableDataReader::requireRestoring() const
{
  if (!_is_restoring)
    mooseError(
        "The RestartableDataReader is not available for querying as it is not currently restoring");
  mooseAssert(_streams.header, "Header not available");
  mooseAssert(_streams.data, "Data not available");
}

bool
RestartableDataReader::isSameType(const RestartableDataReader::HeaderEntry & header_entry,
                                  const std::type_info & type) const
{
#ifndef RESTARTABLE_SKIP_CHECK_HASH_CODE
  if (header_entry.type_hash_code == type.hash_code())
    return true;
#endif
  return header_entry.type == type.name();
}

RestartableDataValue &
RestartableDataReader::restoreData(const std::string & data_name,
                                   std::unique_ptr<RestartableDataValue> value,
                                   const THREAD_ID tid)
{
  auto & data_map = currentData(tid);
  if (data_map.hasData(data_name))
    mooseError("RestartableDataReader::restoreData(): Cannot declare restartable data '",
               data_name,
               "' because it has already been declared");

  const auto & header = getHeader(data_name, tid);
  auto & added_value = currentData(tid).addData(std::move(value));
  deserializeValue(*_streams.data, added_value, header);
  return added_value;
}
