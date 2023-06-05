//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataIO.h"

#include "RestartableDataMap.h"
#include "MooseApp.h"
#include "DataIO.h"
#include "MooseUtils.h"

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <typeinfo>

const std::string RestartableDataIO::RESTARTABLE_DATA_EXT = ".rd";
const unsigned int RestartableDataIO::CURRENT_BACKUP_FILE_VERSION = 3;

RestartableDataIO::RestartableDataIO(MooseApp & moose_app)
  : PerfGraphInterface(moose_app.perfGraph(), "RestartableDataIO"), _moose_app(moose_app)
{
}

std::shared_ptr<Backup>
RestartableDataIO::createBackup()
{
  std::shared_ptr<Backup> backup = std::make_shared<Backup>();

  const auto & restartable_data_maps = _moose_app.getRestartableData();

  for (const auto tid : make_range(libMesh::n_threads()))
    serializeRestartableData(restartable_data_maps[tid], backup->data(tid, {}));

  return backup;
}

void
RestartableDataIO::setBackup(std::shared_ptr<Backup> backup)
{
  _backup = backup;
}

void
RestartableDataIO::clearBackup()
{
  _backup.reset();
}

void
RestartableDataIO::readBackup(const THREAD_ID tid)
{
  if (!_backup)
    mooseError("RestartableDataIO::readBackup(): Backup not set");

  std::istream stream(_backup->data(tid).rdbuf());
  _backup->dataInfo(tid, {}) = readRestartableData(stream, _backup->filename());
}

std::unordered_map<std::string, Backup::DataInfo>
RestartableDataIO::readRestartableData(std::istream & stream,
                                       const std::string & filename /* = "" */) const
{
  auto error = [&filename](const std::string & prefix, const std::string & suffix = "")
  {
    std::stringstream err;
    err << prefix;
    if (filename.size())
      err << " in file '" << filename << "'";
    if (suffix.size())
      err << "\n\n" << suffix;
    mooseError(err.str());
  };

  std::unordered_map<std::string, Backup::DataInfo> data_map;

  // rewind to the beginning
  stream.seekg(0);

  // ID
  char this_id[2];
  stream.read(this_id, 2);
  if (this_id[0] != 'R' || this_id[1] != 'D')
    error("Loaded backup is invalid or corrupted (unexpected header)");

  // File version
  std::remove_const<decltype(CURRENT_BACKUP_FILE_VERSION)>::type this_file_version;
  dataLoad(stream, this_file_version, nullptr);
  if (this_file_version != CURRENT_BACKUP_FILE_VERSION)
    error("Loaded mismatching backup version",
          "  Current version: " + std::to_string(CURRENT_BACKUP_FILE_VERSION) +
              "\n  Loaded version: " + std::to_string(this_file_version));

  // Type id for a basic type
  std::size_t this_compare_hash_code;
  dataLoad(stream, this_compare_hash_code, nullptr);
  if (this_compare_hash_code != typeid(COMPARE_HASH_CODE_TYPE).hash_code())
    error("Loaded backup is not compatible",
          "The hash code check for a basic type (" +
              MooseUtils::prettyCppType<COMPARE_HASH_CODE_TYPE>() +
              ") failed; it is likely that this backup was stored with a different architecture or "
              "operating system");

  // Number of procs
  processor_id_type this_n_procs = 0;
  dataLoad(stream, this_n_procs, nullptr);
  if (_error_on_different_number_of_processors && this_n_procs != _moose_app.n_processors())
    error("Mismatching number of MPI ranks in backup",
          "  Current ranks: " + std::to_string(_moose_app.n_processors()) +
              "\n  Loaded ranks: " + std::to_string(this_n_procs));

  // Number of threads
  unsigned int this_n_threads = 0;
  dataLoad(stream, this_n_threads, nullptr);
  if (_error_on_different_number_of_threads && this_n_threads != libMesh::n_threads())
    error("Mismatching number of threads in backup",
          "  Current threads: " + std::to_string(libMesh::n_threads()) +
              "\n  Loaded threads: " + std::to_string(this_n_threads));

  // Number of RestartableData
  std::size_t n_data = 0;
  dataLoad(stream, n_data, nullptr);

  // Data header block size; we store/load this so that we know the positions
  // of the data at the time of the loop below
  std::size_t n_header_data = 0;
  dataLoad(stream, n_header_data, nullptr);

  // Data header
  auto current_data_position = stream.tellg();
  current_data_position += n_header_data;
  for (const auto i : make_range(n_data))
  {
    std::ignore = i;

    std::string name;
    dataLoad(stream, name, nullptr);
    mooseAssert(name.size(), "Empty name");

    mooseAssert(!data_map.count(name), "Data '" + name + "' is already inserted");
    auto & entry = data_map[name];

    dataLoad(stream, entry.size, nullptr);
    dataLoad(stream, entry.type_hash_code, nullptr);
    dataLoad(stream, entry.type, nullptr);
    entry.position = current_data_position;

    current_data_position += entry.size;
  }

  return data_map;
}

void
RestartableDataIO::restoreBackup(bool for_restart)
{
  auto & restartable_data_maps = _moose_app.getRestartableData();

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto & data_info = _backup->dataInfo(tid, {});
    if (data_info.empty())
      readBackup(tid);

    std::istream stream(_backup->data(tid).rdbuf());

    // When doing restart - make sure we don't read data that is only for recovery
    if (for_restart)
      deserializeRestartableData(
          restartable_data_maps[tid], stream, data_info, _moose_app.getRecoverableData());
    else
      deserializeRestartableData(restartable_data_maps[tid], stream, data_info, DataNames());

    // Now that we've loaded everything, clear the load list
    // TODO: move this somewhere else!
    for (auto & entry : data_info)
      entry.second.loaded = false;
  }
}

void
RestartableDataIO::restoreData(const std::string & name, const THREAD_ID tid)
{
  auto & restartable_data_maps = _moose_app.getRestartableData();

  auto & data_info = _backup->dataInfo(tid, {});
  if (data_info.empty())
    readBackup(tid);

  RestartableDataValue * value = restartable_data_maps[tid].findData(name);
  if (!value)
    mooseError("RestartableDataIO::restoreData(): RestartableData with name '",
               name,
               "' was not declared");

  auto find_data = data_info.find(name);
  if (find_data == data_info.end())
    mooseError(
        "RestartableDataIO::restoreData(): RestartableData with name '", name, "' was not stored");

  std::istream stream(_backup->data(tid).rdbuf());
  auto & data_entry = find_data->second;
  deserializeRestartableDataValue(*value, data_entry, stream);
}

void
RestartableDataIO::writeRestartableData(const std::string & file_name,
                                        const RestartableDataMap & restartable_data)

{
  std::ofstream out;

  out.open(file_name.c_str(), std::ios::out | std::ios::binary);
  if (out.fail())
    mooseError("Unable to open file ", file_name);

  serializeRestartableData(restartable_data, out);

  out.close();
}

void
RestartableDataIO::readRestartableData(const std::string & file_name,
                                       RestartableDataMap & restartable_data,
                                       const DataNames & filter_names)

{
  std::ifstream in;

  in.open(file_name.c_str(), std::ios::in | std::ios::binary);
  if (in.fail())
    mooseError("Unable to open file ", file_name);

  auto data_map = readRestartableData(in, file_name);
  deserializeRestartableData(restartable_data, in, data_map, filter_names);

  in.close();
}

void
RestartableDataIO::serializeRestartableData(const RestartableDataMap & restartable_data,
                                            std::ostream & stream)

{
  // Header
  char id[] = {'R', 'D'};
  stream.write(id, 2);

  // File version
  auto file_version = CURRENT_BACKUP_FILE_VERSION;
  dataStore(stream, file_version, nullptr);

  // Type hash for basic hash
  std::size_t compare_hash_code = typeid(COMPARE_HASH_CODE_TYPE).hash_code();
  dataStore(stream, compare_hash_code, nullptr);

  // Number of procs
  processor_id_type n_procs = _moose_app.n_processors();
  dataStore(stream, n_procs, nullptr);

  // Number of threads
  unsigned int n_threads = libMesh::n_threads();
  dataStore(stream, n_threads, nullptr);

  // Number of RestartableData
  std::size_t n_data = restartable_data.size();
  dataStore(stream, n_data, nullptr);

  // Write out the RestartableData header, and store the actual data separately
  std::ostringstream data_header_blk;
  std::ostringstream data_blk;
  for (const auto & data : restartable_data)
  {
    // Store the data separately, to be added later
    std::ostringstream data_stream;
    data->store(data_stream);
    data_stream.seekp(0, std::ios::end);

    // Append data to the full block
    data_blk << data_stream.str();

    // Store name, size, type hash, and type in the header
    mooseAssert(data->name().size(), "Empty name");
    std::string name = data->name();
    std::size_t data_size = static_cast<std::size_t>(data_stream.tellp());
    std::string type = data->typeId().name();
    std::size_t type_hash_code = data->typeId().hash_code();
    dataStore(data_header_blk, name, nullptr);
    dataStore(data_header_blk, data_size, nullptr);
    dataStore(data_header_blk, type_hash_code, nullptr);
    dataStore(data_header_blk, type, nullptr);
  }

  // size of data header block
  std::size_t data_header_size = static_cast<std::size_t>(data_header_blk.tellp());
  dataStore(stream, data_header_size, nullptr);

  // Write data header and then the data
  stream << data_header_blk.str();
  stream << data_blk.str();
}

void
RestartableDataIO::deserializeRestartableDataValue(RestartableDataValue & value,
                                                   Backup::DataInfo & data_entry,
                                                   std::istream & stream)
{
  if (data_entry.type_hash_code != value.typeId().hash_code() &&
      data_entry.type != value.typeId().name())
    mooseError("Type mismatch for loading RestartableData '",
               value.name(),
               "\n\n  Stored type: ",
               data_entry.type,
               "\n  Declared type: ",
               value.typeId().name(),
               "\n  Stored type hash code: ",
               data_entry.type_hash_code,
               "\n  Declared type hash code: ",
               value.typeId().hash_code());

  // We loaded this earlier
  if (data_entry.loaded)
    return;

  stream.seekg(data_entry.position);
  value.load(stream);

  if (stream.tellg() == -1)
    mooseError("Failed to load RestartableData '", value.name(), "' of type '", value.type(), "'");

  if ((data_entry.position + (std::streampos)data_entry.size) != stream.tellg())
    mooseError("Size mismatch for loading RestartableData '",
               value.name(),
               "' of type '",
               value.type(),
               "'\n\n  Stored size: ",
               data_entry.size,
               "\n  Loaded size: ",
               stream.tellg() - data_entry.position);

  data_entry.loaded = true;
}

void
RestartableDataIO::deserializeRestartableData(
    RestartableDataMap & restartable_data,
    std::istream & stream,
    std::unordered_map<std::string, Backup::DataInfo> & data_map,
    const DataNames & filter_names)
{
  const auto recovering = _moose_app.isRecovering();

  std::vector<std::string> missing_data;
  std::vector<std::string> ignored_data;

  // Load the data in the order that it was requested
  for (auto & data : restartable_data)
  {
    const auto & name = data->name();

    auto find_data = data_map.find(name);
    if (find_data == data_map.end())
    {
      missing_data.push_back(name);
      continue;
    }
    auto & data_entry = find_data->second;

    // Only restore values if we're either recovering or the data isn't filtered out
    const auto is_data_in_filter = filter_names.find(name) != filter_names.end();
    if (recovering || !is_data_in_filter)
      deserializeRestartableDataValue(*data, data_entry, stream);
    // Skip this piece of data and do not report if restarting and recoverable data is not used
    else
    {
      if (recovering && !is_data_in_filter)
        ignored_data.push_back(name);
    }
  }

  // Produce a warning if restarting and restart data is being skipped
  // Do not produce the warning with recovery b/c in cases the parent defines a something as
  // recoverable,
  // but only certain child classes use the value in recovery (i.e., FileOutput::_num_files is
  // needed by Exodus but not Checkpoint)
  if (ignored_data.size() && !recovering)
  {
    std::ostringstream names;
    for (unsigned int i = 0; i < ignored_data.size(); i++)
      names << ignored_data[i] << "\n";
    mooseWarning("The following RestartableData was found in restart file but is being ignored:\n",
                 names.str());
  }
}
