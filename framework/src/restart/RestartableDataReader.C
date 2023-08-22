//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataReader.h"

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
RestartableDataReader::setInput(std::shared_ptr<std::istream> stream)
{
  mooseAssert(stream, "Invalid stream");
  mooseAssert(!_stream, "Already set");
  _stream = stream;
}

void
RestartableDataReader::clear()
{
  _stream.reset();
  _header.clear();
}

void
RestartableDataReader::readHeader()
{
  mooseAssert(_header.empty(), "Header exists");
  _header.clear();

  mooseAssert(_stream, "Not set");
  auto & stream = *_stream;

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
          "\nLoaded version: " + this_file_version);

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

  _header.resize(dataSize());

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

      mooseAssert(!_header[tid].count(name), "Data '" + name + "' is already inserted");
      auto & entry = _header[tid][name];

      dataLoad(stream, entry.size, nullptr);
      dataLoad(stream, entry.type_hash_code, nullptr);
      dataLoad(stream, entry.type, nullptr);
      entry.position = current_data_position;

      current_data_position += entry.size;
    }
  }

  stream.seekg(0);
}

void
RestartableDataReader::restore(const bool retain, const DataNames & filter_names /* = {} */)
{
  mooseAssert(_stream, "Not set");

  readHeader();

  for (const auto tid : index_range(_header))
    deserialize(tid, filter_names);

  if (!retain)
    clear();
}

void
RestartableDataReader::restore(const std::string & file_name,
                               const bool retain,
                               const DataNames & filter_names /* = {} */)
{
  mooseAssert(_header.empty(), "Data exists");

  std::shared_ptr<std::istream> stream = std::make_unique<std::ifstream>();
  setInput(stream);

  auto in = static_cast<std::ifstream *>(stream.get());
  in->open(file_name.c_str(), std::ios::in | std::ios::binary);
  if (in->fail())
    mooseError("RestartableDataReader::restore(): Unable to open file '", file_name, "'");

  restore(retain, filter_names);
}

void
RestartableDataReader::deserializeValue(RestartableDataValue & value,
                                        const RestartableDataReader::HeaderEntry & header_entry)
{
  mooseAssert(_stream, "Not set");
  auto & stream = *_stream;

  if (
#ifndef RESTARTABLE_SKIP_CHECK_HASH_CODE
      header_entry.type_hash_code != value.typeId().hash_code() &&
#endif
      header_entry.type != value.typeId().name())
    mooseError("Type mismatch for loading RestartableData '",
               value.name(),
               "\n\nStored type: ",
               header_entry.type,
               "\nDeclared type: ",
               value.typeId().name(),
               "\nStored type hash code: ",
               header_entry.type_hash_code,
               "\nDeclared type hash code: ",
               value.typeId().hash_code());

  stream.seekg(header_entry.position);
  value.load(stream);

  if (stream.tellg() == -1)
    mooseError("Failed to load RestartableData '", value.name(), "' of type '", value.type(), "'");

  if ((header_entry.position + (std::streampos)header_entry.size) != stream.tellg())
    mooseError("Size mismatch for loading RestartableData '",
               value.name(),
               "' of type '",
               value.type(),
               "'\n\nStored size: ",
               header_entry.size,
               "\nLoaded size: ",
               stream.tellg() - header_entry.position);
}

void
RestartableDataReader::deserialize(const THREAD_ID tid, const DataNames & filter_names)
{
  mooseAssert(_stream, "Not set");

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
      deserializeValue(*value, header_entry);
  }
}
