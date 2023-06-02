//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RestartableDataIO.h"

#include "Backup.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseUtils.h"
#include "DataIO.h"

#include <stdio.h>
#include <fstream>
#include <sstream>

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
    serializeRestartableData(restartable_data_maps[tid], backup->restartableData(tid, {}));

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

  std::istream stream(_backup->restartableData(tid).rdbuf());
  _backup->restartableDataMap(tid, {}) = readRestartableData(stream);
}

std::unordered_map<std::string, Backup::DataEntry>
RestartableDataIO::readRestartableData(std::istream & stream) const
{
  std::unordered_map<std::string, Backup::DataEntry> data_map;

  // rewind to the beginning
  stream.seekg(0);

  // header
  char id[2];
  stream.read(id, 2);

  unsigned int this_file_version;
  stream.read((char *)&this_file_version, sizeof(this_file_version));

  if (this_file_version != CURRENT_BACKUP_FILE_VERSION)
    mooseError("Mismatching backup file version! ",
               "Current version: ",
               CURRENT_BACKUP_FILE_VERSION,
               " Version in file: ",
               this_file_version);

  processor_id_type this_n_procs = 0;
  unsigned int this_n_threads = 0;

  stream.read((char *)&this_n_procs, sizeof(this_n_procs));
  stream.read((char *)&this_n_threads, sizeof(this_n_threads));

  if (_error_on_different_number_of_processors && this_n_procs != _moose_app.n_processors())
    mooseError("Mismatching number of MPI ranks in backup file! ",
               "Current run: ",
               _moose_app.n_processors(),
               " In file: ",
               this_n_procs);

  if (_error_on_different_number_of_threads && this_n_threads != libMesh::n_threads())
    mooseError("Mismatching number of threads in backup file! ",
               "Current run: ",
               libMesh::n_threads(),
               " In file: ",
               this_n_threads);

  // Number of data
  std::size_t n_data = 0;
  stream.read((char *)&n_data, sizeof(n_data));
  // Data header block size; we store/load this so that we know the positions
  // of the data at the time of the loop below
  std::size_t n_header_data = 0;
  stream.read((char *)&n_header_data, sizeof(n_header_data));
  // Data header
  auto current_data_position = stream.tellg();
  current_data_position += n_header_data;
  for (const auto i : make_range(n_data))
  {
    std::ignore = i;

    std::string name;
    dataLoad(stream, name, nullptr);

    mooseAssert(!data_map.count(name), "Already inserted");
    auto & entry = data_map[name];

    dataLoad(stream, entry.size, nullptr);
    dataLoad(stream, entry.type_hash_code, nullptr);
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
    auto & data_map = _backup->restartableDataMap(0, {});
    if (data_map.empty())
      readBackup(tid);

    std::istream stream(_backup->restartableData(tid).rdbuf());

    // When doing restart - make sure we don't read data that is only for recovery
    if (for_restart)
      deserializeRestartableData(
          restartable_data_maps[tid], stream, data_map, _moose_app.getRecoverableData());
    else
      deserializeRestartableData(restartable_data_maps[tid], stream, data_map, DataNames());

    // Now that we've loaded everything, clear the load list
    // TODO: move this somewhere else!
    for (auto & entry : data_map)
      entry.second.loaded = false;
  }
}

void
RestartableDataIO::restoreData(const std::string & name)
{
  auto & restartable_data_maps = _moose_app.getRestartableData();

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto & data_map = _backup->restartableDataMap(0, {});
    if (data_map.empty())
      readBackup(tid);

    RestartableDataValue * value = restartable_data_maps[tid].findData(name);
    if (value == nullptr)
      mooseError("RestartableDataIO::restoreData(): RestartableData with name '",
                 name,
                 "' was not declared");

    auto find_data = data_map.find(name);
    if (find_data == data_map.end())
      mooseError("RestartableDataIO::restoreData(): RestartableData with name '",
                 name,
                 "' was not stored");

    std::istream stream(_backup->restartableData(tid).rdbuf());
    auto & data_entry = find_data->second;
    deserializeRestartableDataValue(*value, data_entry, stream);
  }
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

  auto data_map = readRestartableData(in);
  deserializeRestartableData(restartable_data, in, data_map, filter_names);

  in.close();
}

void
RestartableDataIO::serializeRestartableData(const RestartableDataMap & restartable_data,
                                            std::ostream & stream)

{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type n_procs = _moose_app.n_processors();

  const unsigned int file_version = CURRENT_BACKUP_FILE_VERSION;

  // Write out header
  char id[] = {'R', 'D'};
  stream.write(id, 2);
  stream.write((const char *)&file_version, sizeof(file_version));
  stream.write((const char *)&n_procs, sizeof(n_procs));
  stream.write((const char *)&n_threads, sizeof(n_threads));

  // number of RestartableData
  std::size_t n_data = restartable_data.size();
  stream.write((const char *)&n_data, sizeof(n_data));

  // Write out the RestartableData header, and store the actual data separately
  std::ostringstream data_header_blk;
  std::ostringstream data_blk;
  for (const auto & data : restartable_data)
  {
    // Store the data separately, to be added later
    std::ostringstream data_stream;
    data->store(data_stream);

    // Append data to the full block
    data_blk << data_stream.str();

    // Store name, size, type hash, and type in the header
    auto name = data->name();
    auto data_size = static_cast<std::size_t>(data_stream.tellp());
    auto hash_code = data->typeId().hash_code();
    dataStore(data_header_blk, name, nullptr);
    dataStore(data_header_blk, data_size, nullptr);
    dataStore(data_header_blk, hash_code, nullptr);
  }

  // size of data header block
  auto data_header_size = static_cast<std::size_t>(data_header_blk.tellp());
  stream.write((const char *)&data_header_size, sizeof(data_header_size));

  // Write data header and then the data
  stream << data_header_blk.str();
  stream << data_blk.str();
}

void
RestartableDataIO::deserializeRestartableDataValue(RestartableDataValue & value,
                                                   Backup::DataEntry & data_entry,
                                                   std::istream & stream)
{
  // Sanity check on types
  if (data_entry.type_hash_code != value.typeId().hash_code())
    mooseError("Type mismatch for loading RestartableData '",
               value.name(),
               "'\n\n  Declared type: ",
               value.type());

  // We loaded this earlier
  if (data_entry.loaded)
    return;

  stream.seekg(data_entry.position);
  value.load(stream);

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
    std::unordered_map<std::string, Backup::DataEntry> & data_map,
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
