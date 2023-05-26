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

#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseUtils.h"

#include <stdio.h>
#include <fstream>

RestartableDataIO::RestartableDataIO(FEProblemBase & fe_problem)
  : RestartableDataIO(fe_problem.getMooseApp(), &fe_problem)
{
}

RestartableDataIO::RestartableDataIO(MooseApp & moose_app, FEProblemBase * fe_problem_ptr)
  : PerfGraphInterface(moose_app.perfGraph(), "RestartableDataIO"),
    _moose_app(moose_app),
    _fe_problem_ptr(fe_problem_ptr)
{
}

std::shared_ptr<Backup>
RestartableDataIO::createBackup()
{
  std::shared_ptr<Backup> backup = std::make_shared<Backup>();

  const auto & restartable_data_maps = _moose_app.getRestartableData();

  unsigned int n_threads = libMesh::n_threads();

  backup->_restartable_data.resize(n_threads);

  for (unsigned int tid = 0; tid < n_threads; tid++)
    serializeRestartableData(restartable_data_maps[tid], *backup->_restartable_data[tid]);

  return backup;
}

void
RestartableDataIO::restoreBackup(std::shared_ptr<Backup> backup, bool for_restart)
{
  unsigned int n_threads = libMesh::n_threads();

  // Make sure we read from the beginning
  for (unsigned int tid = 0; tid < n_threads; tid++)
    backup->_restartable_data[tid]->seekg(0);

  const auto & restartable_data_maps = _moose_app.getRestartableData();

  for (unsigned int tid = 0; tid < n_threads; tid++)
  {
    const auto & recoverable_data_names = _moose_app.getRecoverableData();

    if (for_restart) // When doing restart - make sure we don't read data that is only for recovery
      deserializeRestartableData(
          restartable_data_maps[tid], *backup->_restartable_data[tid], recoverable_data_names);
    else
      deserializeRestartableData(
          restartable_data_maps[tid], *backup->_restartable_data[tid], DataNames());
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

  deserializeRestartableData(restartable_data, in, filter_names);

  in.close();
}

void
RestartableDataIO::serializeRestartableData(const RestartableDataMap & restartable_data,
                                            std::ostream & stream)

{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type n_procs = _moose_app.n_processors();

  const unsigned int file_version = CURRENT_BACKUP_FILE_VERSION;

  { // Write out header
    char id[] = {'R', 'D'};

    stream.write(id, 2);
    stream.write((const char *)&file_version, sizeof(file_version));

    stream.write((const char *)&n_procs, sizeof(n_procs));
    stream.write((const char *)&n_threads, sizeof(n_threads));

    // number of RestartableData
    unsigned int n_data = restartable_data.size();
    stream.write((const char *)&n_data, sizeof(n_data));

    // data names
    for (const auto & it : restartable_data)
    {
      std::string name = it.first;
      stream.write(name.c_str(), name.length() + 1); // trailing 0!
    }
  }
  {
    std::ostringstream data_blk;

    for (const auto & it : restartable_data)
    {
      std::ostringstream data;
      it.second->store(data);

      // Store the size of the data then the data
      unsigned int data_size = static_cast<unsigned int>(data.tellp());
      data_blk.write((const char *)&data_size, sizeof(data_size));
      data_blk << data.str();
    }

    // Write out this proc's block size
    unsigned int data_blk_size = static_cast<unsigned int>(data_blk.tellp());
    stream.write((const char *)&data_blk_size, sizeof(data_blk_size));

    // Write out the values
    stream << data_blk.str();
  }
}

void
RestartableDataIO::deserializeRestartableData(const RestartableDataMap & restartable_data,
                                              std::istream & stream,
                                              const DataNames & filter_names)
{
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

  bool recovering = _moose_app.isRecovering();

  std::vector<std::string> ignored_data;

  // number of data
  unsigned int n_data = 0;
  stream.read((char *)&n_data, sizeof(n_data));

  // data names
  std::vector<std::string> data_names(n_data);

  for (unsigned int i = 0; i < n_data; i++)
  {
    std::getline(stream, data_names[i], '\0');
    if (!stream)
      mooseError("Error while reading stream");
  }

  // Grab this processor's block size
  unsigned int data_blk_size = 0;
  stream.read((char *)&data_blk_size, sizeof(data_blk_size));

  for (unsigned int i = 0; i < n_data; i++)
  {
    std::string current_name = data_names[i];

    unsigned int data_size = 0;
    stream.read((char *)&data_size, sizeof(data_size));

    // Determine if the current name is in the filter set
    bool is_data_restartable = restartable_data.find(current_name) != restartable_data.end();
    bool is_data_in_filter = filter_names.find(current_name) != filter_names.end();
    if (is_data_restartable      // Only restore values if they're currently being used and
        && (recovering ||        // Only read this value if we're either recovering or
            !is_data_in_filter)) // the data isn't specifically filtered out
    {
      auto current_pair = restartable_data.find(current_name);
      if (current_pair == restartable_data.end())
        mooseError("restartable_data missing ", current_name, "\n");
      current_pair->second->load(stream);
    }
    else
    {
      // Skip this piece of data and do not report if restarting and recoverable data is not used
      stream.seekg(data_size, std::ios_base::cur);
      if (recovering && !is_data_in_filter)
        ignored_data.push_back(current_name);
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

