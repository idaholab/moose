/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "RestartableDataIO.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseUtils.h"
#include "NonlinearSystem.h"
#include "RestartableData.h"

#include <stdio.h>

RestartableDataIO::RestartableDataIO(FEProblemBase & fe_problem) : _fe_problem(fe_problem)
{
  _in_file_handles.resize(libMesh::n_threads());
}

void
RestartableDataIO::writeRestartableData(std::string base_file_name,
                                        const RestartableDatas & restartable_datas,
                                        std::set<std::string> & /*_recoverable_data*/)
{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type proc_id = _fe_problem.processor_id();

  for (unsigned int tid = 0; tid < n_threads; tid++)
  {
    std::ofstream out;

    std::ostringstream file_name_stream;
    file_name_stream << base_file_name;

    file_name_stream << "-" << proc_id;

    if (n_threads > 1)
      file_name_stream << "-" << tid;

    std::string file_name = file_name_stream.str();
    out.open(file_name.c_str(), std::ios::out | std::ios::binary);

    serializeRestartableData(restartable_datas[tid], out);

    out.close();
  }
}

void
RestartableDataIO::serializeRestartableData(
    const std::map<std::string, RestartableDataValue *> & restartable_data, std::ostream & stream)
{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type n_procs = _fe_problem.n_processors();

  const unsigned int file_version = 2;

  { // Write out header
    char id[2];

    // header
    id[0] = 'R';
    id[1] = 'D';

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
RestartableDataIO::deserializeRestartableData(
    const std::map<std::string, RestartableDataValue *> & restartable_data,
    std::istream & stream,
    const std::set<std::string> & recoverable_data)
{
  bool recovering = _fe_problem.getMooseApp().isRecovering();

  std::vector<std::string> ignored_data;

  // number of data
  unsigned int n_data = 0;
  stream.read((char *)&n_data, sizeof(n_data));

  // data names
  std::vector<std::string> data_names(n_data);

  for (unsigned int i = 0; i < n_data; i++)
  {
    std::string data_name;
    char ch = 0;
    do
    {
      stream.read(&ch, 1);
      if (ch != '\0')
        data_name += ch;
    } while (ch != '\0');
    data_names[i] = data_name;
  }

  // Grab this processor's block size
  unsigned int data_blk_size = 0;
  stream.read((char *)&data_blk_size, sizeof(data_blk_size));

  for (unsigned int i = 0; i < n_data; i++)
  {
    std::string current_name = data_names[i];

    unsigned int data_size = 0;
    stream.read((char *)&data_size, sizeof(data_size));

    // Determine if the current data is recoverable
    bool is_data_restartable = restartable_data.find(current_name) != restartable_data.end();
    bool is_data_recoverable = recoverable_data.find(current_name) != recoverable_data.end();
    if (is_data_restartable // Only restore values if they're currently being used
        &&
        (recovering || !is_data_recoverable)) // Only read this value if we're either recovering or
                                              // this hasn't been specified to be recovery only data

    {
      // Moose::out<<"Loading "<<current_name<<std::endl;

      try
      {
        RestartableDataValue * current_data = restartable_data.at(current_name);
        current_data->load(stream);
      }
      catch (...)
      {
        mooseError("restartable_data missing ", current_name, "\n");
      }
    }
    else
    {
      // Skip this piece of data and do not report if restarting and recoverable data is not used
      stream.seekg(data_size, std::ios_base::cur);
      if (recovering && !is_data_recoverable)
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

void
RestartableDataIO::serializeSystems(std::ostream & stream)
{
  storeHelper(stream, static_cast<SystemBase &>(_fe_problem.getNonlinearSystemBase()), NULL);
  storeHelper(stream, static_cast<SystemBase &>(_fe_problem.getAuxiliarySystem()), NULL);
}

void
RestartableDataIO::deserializeSystems(std::istream & stream)
{
  loadHelper(stream, static_cast<SystemBase &>(_fe_problem.getNonlinearSystemBase()), NULL);
  loadHelper(stream, static_cast<SystemBase &>(_fe_problem.getAuxiliarySystem()), NULL);
}

void
RestartableDataIO::readRestartableDataHeader(std::string base_file_name)
{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type n_procs = _fe_problem.n_processors();
  processor_id_type proc_id = _fe_problem.processor_id();

  for (unsigned int tid = 0; tid < n_threads; tid++)
  {
    std::ostringstream file_name_stream;
    file_name_stream << base_file_name;
    file_name_stream << "-" << proc_id;

    if (n_threads > 1)
      file_name_stream << "-" << tid;

    std::string file_name = file_name_stream.str();

    MooseUtils::checkFileReadable(file_name);

    const unsigned int file_version = 2;

    _in_file_handles[tid] =
        std::make_shared<std::ifstream>(file_name.c_str(), std::ios::in | std::ios::binary);

    // header
    char id[2];
    _in_file_handles[tid]->read(id, 2);

    unsigned int this_file_version;
    _in_file_handles[tid]->read((char *)&this_file_version, sizeof(this_file_version));

    processor_id_type this_n_procs = 0;
    unsigned int this_n_threads = 0;

    _in_file_handles[tid]->read((char *)&this_n_procs, sizeof(this_n_procs));
    _in_file_handles[tid]->read((char *)&this_n_threads, sizeof(this_n_threads));

    // check the header
    if (id[0] != 'R' || id[1] != 'D')
      mooseError("Corrupted restartable data file!");

    // check the file version
    if (this_file_version > file_version)
      mooseError("Trying to restart from a newer file version - you need to update MOOSE");

    if (this_file_version < file_version)
      mooseError("Trying to restart from an older file version - you need to checkout an older "
                 "version of MOOSE.");

    if (this_n_procs != n_procs)
      mooseError("Cannot restart using a different number of processors!");

    if (this_n_threads != n_threads)
      mooseError("Cannot restart using a different number of threads!");
  }
}

void
RestartableDataIO::readRestartableData(const RestartableDatas & restartable_datas,
                                       const std::set<std::string> & recoverable_data)
{
  unsigned int n_threads = libMesh::n_threads();
  std::vector<std::string> ignored_data;

  for (unsigned int tid = 0; tid < n_threads; tid++)
  {
    const std::map<std::string, RestartableDataValue *> & restartable_data = restartable_datas[tid];

    if (!_in_file_handles[tid].get() || !_in_file_handles[tid]->is_open())
      mooseError("In RestartableDataIO: Need to call readRestartableDataHeader() before calling "
                 "readRestartableData()");

    deserializeRestartableData(restartable_data, *_in_file_handles[tid], recoverable_data);

    _in_file_handles[tid]->close();
  }
}

std::shared_ptr<Backup>
RestartableDataIO::createBackup()
{
  std::shared_ptr<Backup> backup = std::make_shared<Backup>();

  serializeSystems(backup->_system_data);

  const RestartableDatas & restartable_datas = _fe_problem.getMooseApp().getRestartableData();

  unsigned int n_threads = libMesh::n_threads();

  backup->_restartable_data.resize(n_threads);

  for (unsigned int tid = 0; tid < n_threads; tid++)
    serializeRestartableData(restartable_datas[tid], *backup->_restartable_data[tid]);

  return backup;
}

void
RestartableDataIO::restoreBackup(std::shared_ptr<Backup> backup, bool for_restart)
{
  unsigned int n_threads = libMesh::n_threads();

  // Make sure we read from the beginning
  backup->_system_data.seekg(0);
  for (unsigned int tid = 0; tid < n_threads; tid++)
    backup->_restartable_data[tid]->seekg(0);

  deserializeSystems(backup->_system_data);

  const RestartableDatas & restartable_datas = _fe_problem.getMooseApp().getRestartableData();

  for (unsigned int tid = 0; tid < n_threads; tid++)
  {
    // header
    char id[2];
    backup->_restartable_data[tid]->read(id, 2);

    unsigned int this_file_version;
    backup->_restartable_data[tid]->read((char *)&this_file_version, sizeof(this_file_version));

    processor_id_type this_n_procs = 0;
    unsigned int this_n_threads = 0;

    backup->_restartable_data[tid]->read((char *)&this_n_procs, sizeof(this_n_procs));
    backup->_restartable_data[tid]->read((char *)&this_n_threads, sizeof(this_n_threads));

    std::set<std::string> & recoverable_data = _fe_problem.getMooseApp().getRecoverableData();

    if (for_restart) // When doing restart - make sure we don't read data that is only for
                     // recovery...
      deserializeRestartableData(
          restartable_datas[tid], *backup->_restartable_data[tid], recoverable_data);
    else
      deserializeRestartableData(
          restartable_datas[tid], *backup->_restartable_data[tid], std::set<std::string>());
  }
}
