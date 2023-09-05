//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataWriter.h"

#include "DataIO.h"
#include "RestartableDataMap.h"

#include <fstream>
#include <system_error>

RestartableDataWriter::RestartableDataWriter(MooseApp & app, RestartableDataMap & data)
  : RestartableDataIO(app, data)
{
}

RestartableDataWriter::RestartableDataWriter(MooseApp & app, std::vector<RestartableDataMap> & data)
  : RestartableDataIO(app, data)
{
}

void
RestartableDataWriter::write(std::ostream & header_stream, std::ostream & data_stream)
{
  // Set everything as not stored
  for (const auto tid : make_range(dataSize()))
    for (auto & value : currentData(tid))
      value.setNotStored({});

  // Header
  char id[] = {'R', 'D'};
  header_stream.write(id, 2);

  // File version
  auto file_version = CURRENT_BACKUP_FILE_VERSION;
  dataStore(header_stream, file_version, nullptr);

  // Type hash for basic hash
  std::size_t compare_hash_code = typeid(COMPARE_HASH_CODE_TYPE).hash_code();
  dataStore(header_stream, compare_hash_code, nullptr);

  // Number of procs
  auto n_procs = n_processors();
  dataStore(header_stream, n_procs, nullptr);

  // Number of data
  auto num_data = dataSize();
  dataStore(header_stream, num_data, nullptr);

  // Size of data for each thread
  for (const auto tid : make_range(dataSize()))
  {
    std::size_t n_data = currentData(tid).size();
    dataStore(header_stream, n_data, nullptr);
  }

  // Write out the RestartableData header, and store the actual data separately
  for (const auto tid : make_range(dataSize()))
    for (auto & data : currentData(tid))
    {
      // Store the data
      const std::size_t data_start_position = static_cast<std::size_t>(data_stream.tellp());
      data.store(data_stream);
      std::size_t data_size = static_cast<std::size_t>(data_stream.tellp()) - data_start_position;

      // Store name, size, type hash, and type in the header
      mooseAssert(data.name().size(), "Empty name");
      std::string name = data.name();
      std::string type = data.typeId().name();
      std::size_t type_hash_code = data.typeId().hash_code();
      bool has_context = data.hasContext();
      dataStore(header_stream, name, nullptr);
      dataStore(header_stream, data_size, nullptr);
      dataStore(header_stream, type_hash_code, nullptr);
      dataStore(header_stream, type, nullptr);
      dataStore(header_stream, has_context, nullptr);
    }
}

std::vector<std::filesystem::path>
RestartableDataWriter::write(const std::filesystem::path & folder_base)
{
  const auto header_file = restartableHeaderFile(folder_base);
  const auto data_file = restartableDataFile(folder_base);

  // Make the folder if it doesn't exist
  const auto dir = header_file.parent_path();
  mooseAssert(dir == data_file.parent_path(), "Inconsistent directories");
  if (!std::filesystem::exists(dir))
  {
    std::error_code err;
    if (!std::filesystem::create_directory(dir, err))
      mooseError("Unable to create restart directory\n",
                 std::filesystem::absolute(dir),
                 "\n\n",
                 err.message());
  }

  // We want to keep track of the paths that we create so that we can
  // return them for file management later (primarily removing old checkpoints)
  std::vector<std::filesystem::path> paths;

  // Helper for opening a stream for use
  auto open = [&paths](const std::filesystem::path & filename)
  {
    std::ofstream stream;
    stream.open(filename.c_str(), std::ios::out | std::ios::binary);
    if (!stream.is_open())
      mooseError(
          "Unable to open restart file ", std::filesystem::absolute(filename), " for writing");
    paths.push_back(filename);
    return stream;
  };

  // Open and write
  auto header_stream = open(header_file);
  auto data_stream = open(data_file);
  write(header_stream, data_stream);

  // Update the folder's modified time to now for consistency
  // (this used in checking for the latest checkpoint)
  const auto data_write_time = std::filesystem::last_write_time(header_file);
  std::filesystem::last_write_time(dir, data_write_time);

  return paths;
}
