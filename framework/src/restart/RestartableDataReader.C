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

#include <fstream>

// Type hash codes can't be relied on for older clang...
// not sure why, and also don't care why
#if defined(__clang__) && __clang_major__ < 12
#define RESTARTABLE_SKIP_CHECK_HASH_CODE
#endif

RestartableDataReader::RestartableDataReader(MooseApp & app, RestartableDataMap & data)
  : RestartableDataIO(app, data)
{
}

RestartableDataReader::RestartableDataReader(MooseApp & app, std::vector<RestartableDataMap> & data)
  : RestartableDataIO(app, data)
{
}

void
RestartableDataReader::setInput(std::unique_ptr<std::stringstream> stream)
{
  mooseAssert(!_input, "Input already set");
  _input = std::make_unique<StringInputStream>(std::move(stream));
}

void
RestartableDataReader::setInput(const std::string & filename)
{
  mooseAssert(!_input, "Input already set");
  _input = std::make_unique<FileInputStream>(filename);
}

std::unique_ptr<InputStream>
RestartableDataReader::clear()
{
  _header.clear();
  return std::move(_input);
}

std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>>
RestartableDataReader::readHeader(std::istream & stream) const
{
  std::vector<std::unordered_map<std::string, RestartableDataReader::HeaderEntry>> header;

  stream.seekg(0);

  const auto error = [](auto... args)
  { mooseError("RestartableDataReader::readHeader(): ", args...); };

  // ID
  char this_id[2];
  stream.read(this_id, 2);
  if (this_id[0] != 'R' || this_id[1] != 'D')
    error("Loaded backup is invalid or corrupted (unexpected header)");

  // File version
  std::remove_const<decltype(CURRENT_BACKUP_FILE_VERSION)>::type this_file_version;
  dataLoad(stream, this_file_version, nullptr);
  if (this_file_version != CURRENT_BACKUP_FILE_VERSION)
    error("Loaded mismatching backup version\n\n",
          "Current version: ",
          CURRENT_BACKUP_FILE_VERSION,
          "\nLoaded version: ",
          this_file_version);

  // Type id for a basic type
  std::size_t this_compare_hash_code;
  dataLoad(stream, this_compare_hash_code, nullptr);
#ifndef RESTARTABLE_SKIP_CHECK_HASH_CODE
  if (this_compare_hash_code != typeid(COMPARE_HASH_CODE_TYPE).hash_code())
    error("Loaded backup is not compatible\n\nThe hash code check for a basic type (",
          MooseUtils::prettyCppType<COMPARE_HASH_CODE_TYPE>(),
          ") failed.\nIt is likely that this backup was stored with a different architecture or "
          "operating system");
#endif

  // Number of procs
  decltype(n_processors()) this_n_procs = 0;
  dataLoad(stream, this_n_procs, nullptr);
  if (_error_on_different_number_of_processors && this_n_procs != n_processors())
    error("Mismatching number of MPI ranks in backup\n\nCurrent ranks: ",
          n_processors(),
          "\nLoaded ranks: ",
          this_n_procs);

  // Number of data
  decltype(dataSize()) this_num_data = 0;
  dataLoad(stream, this_num_data, nullptr);
  if (this_num_data != dataSize())
    error("Mismatching number of threads in backup\n\nCurrent threads: ",
          dataSize(),
          "\nLoaded threads: ",
          this_num_data);

  header.resize(dataSize());

  // Size of data for each thread
  std::vector<std::size_t> tid_n_data(dataSize());
  for (const auto tid : make_range(dataSize()))
    dataLoad(stream, tid_n_data[tid], nullptr);

  // Data header block size; we store/load this so that we know the positions
  // of the data at the time of the loop below
  std::size_t n_header_data = 0;
  dataLoad(stream, n_header_data, nullptr);

  // Load for each thread
  for (const auto tid : make_range(dataSize()))
  {
    // Data header
    auto current_data_position = stream.tellg();
    current_data_position += n_header_data;
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
      entry.position = current_data_position;

      current_data_position += entry.size;
    }
  }

  stream.seekg(0);

  return header;
}

void
RestartableDataReader::restore(const DataNames & filter_names /* = {} */)
{
  if (!_input)
    mooseError("RestartableDataReader::restore(): Cannot restore because an input was not set");
  if (_header.size())
    mooseError(
        "RestartableDataReader::restore(): Cannot restore because old data exists; call clear()");

  // This gives us ownership of a new input stream
  auto stream_ptr = _input->get();
  auto & stream = *stream_ptr;

  _header = readHeader(stream);

  for (const auto tid : index_range(_header))
  {
    auto & data = currentData(tid);
    const auto & header = _header[tid];

    // TODO: Think about what to do with missing data

    // Load the data in the order that it was requested
    for (auto & value : data.sortedData())
    {
      const auto & name = value->name();

      auto find_header = header.find(name);
      if (find_header == header.end())
        continue;

      auto & header_entry = find_header->second;

      // Only restore values if we're either recovering or the data isn't filtered out
      const auto is_data_in_filter = filter_names.find(name) != filter_names.end();
      if (!is_data_in_filter)
        deserializeValue(stream, *value, header_entry);
    }
  }
}

void
RestartableDataReader::deserializeValue(std::istream & stream,
                                        RestartableDataValue & value,
                                        const RestartableDataReader::HeaderEntry & header_entry)
{
  auto error = [&value](auto... args)
  {
    mooseError("While loading RestartableData '",
               value.name(),
               "' of type '",
               value.type(),
               "':\n\n",
               args...);
  };

  if (
#ifndef RESTARTABLE_SKIP_CHECK_HASH_CODE
      header_entry.type_hash_code != value.typeId().hash_code() &&
#endif
      header_entry.type != value.typeId().name())
    error("The stored type of '", header_entry.type, "' does not match");

  stream.seekg(header_entry.position);
  value.load(stream);

  if (stream.tellg() == -1)
    error("An error was encountered when reading from the stream");

  if ((header_entry.position + (std::streampos)header_entry.size) != stream.tellg())
    error("The data read does not match the data stored");
}
