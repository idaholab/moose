//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataWriter.h"

#include <fstream>

RestartableDataWriter::RestartableDataWriter(MooseApp & app, RestartableDataMap & data)
  : RestartableDataIO(app, data)
{
}

RestartableDataWriter::RestartableDataWriter(MooseApp & app, std::vector<RestartableDataMap> & data)
  : RestartableDataIO(app, data)
{
}

void
RestartableDataWriter::write(std::ostream & stream)
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
  auto n_procs = n_processors();
  dataStore(stream, n_procs, nullptr);

  // Number of data
  auto num_data = dataSize();
  dataStore(stream, num_data, nullptr);

  // Size of data for each thread
  for (const auto tid : make_range(dataSize()))
  {
    std::size_t n_data = currentData(tid).size();
    dataStore(stream, n_data, nullptr);
  }

  std::ostringstream data_header_blk, data_blk;

  // Write out the RestartableData header, and store the actual data separately
  for (const auto tid : make_range(dataSize()))
    for (const auto & data : currentData(tid).sortedData())
    {
      // Store the data
      const std::size_t data_start_position = static_cast<std::size_t>(data_blk.tellp());
      data->store(data_blk);

      // Store name, size, type hash, and type in the header
      mooseAssert(data->name().size(), "Empty name");
      std::string name = data->name();
      std::size_t data_size = static_cast<std::size_t>(data_blk.tellp()) - data_start_position;
      std::string type = data->typeId().name();
      std::size_t type_hash_code = data->typeId().hash_code();
      dataStore(data_header_blk, name, nullptr);
      dataStore(data_header_blk, data_size, nullptr);
      dataStore(data_header_blk, type_hash_code, nullptr);
      dataStore(data_header_blk, type, nullptr);
    }

  // Store size of data header block
  std::size_t data_header_size = static_cast<std::size_t>(data_header_blk.tellp());
  dataStore(stream, data_header_size, nullptr);

  // Write data header
  stream << data_header_blk.str();
  data_header_blk.clear();

  // ...and then the data
  stream << data_blk.str();
}

void
RestartableDataWriter::write(const std::string & file_name)
{
  std::ofstream stream;
  stream.open(file_name.c_str(), std::ios::out | std::ios::binary);
  if (!stream.is_open())
    mooseError("RestartableDataWriter::write(): Unable to open file '", file_name, "'");

  write(stream);
}
