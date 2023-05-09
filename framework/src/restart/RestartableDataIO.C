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

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseUtils.h"
#include "NonlinearSystem.h"

#include <stdio.h>
#include <fstream>

RestartableDataIO::RestartableDataIO(FEProblemBase & fe_problem)
  : RestartableDataIO(fe_problem.getMooseApp(), &fe_problem)
{
}

RestartableDataIO::RestartableDataIO(MooseApp & moose_app, FEProblemBase * fe_problem_ptr)
  : PerfGraphInterface(moose_app.perfGraph(), "RestartableDataIO"),
    _moose_app(moose_app),
    _fe_problem_ptr(fe_problem_ptr),
    _is_header_read(false),
    _use_binary_ext(true)
{
}

void
RestartableDataIO::writeRestartableDataPerProc(const std::string & base_file_name,
                                               const RestartableDataMaps & restartable_datas)

{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type proc_id = _moose_app.processor_id();

  for (unsigned int tid = 0; tid < n_threads; tid++)
  {
    std::ostringstream file_name_stream;
    file_name_stream << base_file_name;

    file_name_stream << "-" << proc_id;

    if (n_threads > 1)
      file_name_stream << "-" << tid;

    std::string file_name = file_name_stream.str();
    writeRestartableData(file_name, restartable_datas[tid]);
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
RestartableDataIO::serializeRestartableData(const RestartableDataMap & restartable_data,
                                            std::ostream & stream)

{
  unsigned int n_threads = libMesh::n_threads();
  processor_id_type n_procs = _moose_app.n_processors();

  const unsigned int file_version = 2;

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

void
RestartableDataIO::serializeSystems(std::ostream & stream)
{
  mooseAssert(_fe_problem_ptr, "The FEProblem pointer is nullptr in RestartableDataIO");

  for (const auto & nl_sys_num : make_range(_fe_problem_ptr->numNonlinearSystems()))
    storeHelper(stream,
                static_cast<SystemBase &>(_fe_problem_ptr->getNonlinearSystemBase(nl_sys_num)),
                nullptr);
  storeHelper(stream, static_cast<SystemBase &>(_fe_problem_ptr->getAuxiliarySystem()), nullptr);
}

void
RestartableDataIO::deserializeSystems(std::istream & stream)
{
  mooseAssert(_fe_problem_ptr, "The FEProblem pointer is nullptr in RestartableDataIO");

  for (const auto & nl_sys_num : make_range(_fe_problem_ptr->numNonlinearSystems()))
    loadHelper(stream,
               static_cast<SystemBase &>(_fe_problem_ptr->getNonlinearSystemBase(nl_sys_num)),
               nullptr);
  loadHelper(stream, static_cast<SystemBase &>(_fe_problem_ptr->getAuxiliarySystem()), nullptr);
}

bool
RestartableDataIO::readRestartableDataHeader(bool per_proc_id, const std::string & suffix)
{
  const std::string filename =
      _moose_app.getRestartRecoverFileBase() + suffix + RESTARTABLE_DATA_EXT;
  return readRestartableDataHeaderFromFile(filename, per_proc_id);
}

bool
RestartableDataIO::readRestartableDataHeaderFromFile(const std::string & recover_file,
                                                     bool per_proc_id)
{
  unsigned int n_threads = libMesh::n_threads();
  unsigned int n_files = per_proc_id ? n_threads : 1;

  processor_id_type n_procs = _moose_app.n_processors();
  processor_id_type proc_id = _moose_app.processor_id();

  _in_file_handles.resize(n_files);

  for (unsigned int tid = 0; tid < n_files; tid++)
  {
    std::ostringstream file_name_stream;
    file_name_stream << recover_file;

    if (per_proc_id)
    {
      file_name_stream << "-" << proc_id;
      if (n_threads > 1)
        file_name_stream << "-" << tid;
    }

    std::string file_name = file_name_stream.str();

    bool throw_on_error = per_proc_id;
    if (!MooseUtils::checkFileReadable(file_name, false, throw_on_error))
      return false;

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

    if (_error_on_different_number_of_processors && (this_n_procs != n_procs))
      mooseError("Cannot restart using a different number of processors!");

    if (_error_on_different_number_of_threads && (this_n_threads != n_threads))
      mooseError("Cannot restart using a different number of threads!");
  }

  _is_header_read = true;

  return true;
}

void
RestartableDataIO::readRestartableData(const RestartableDataMaps & restartable_datas,
                                       const DataNames & recoverable_data)
{
  TIME_SECTION("readRestartableData", 3, "Reading RestartableData");

  if (!_is_header_read)
    mooseError("In RestartableDataIO: Need to call readRestartableDataHeader() before calling "
               "readRestartableData()");

  for (unsigned int tid = 0; tid < _in_file_handles.size(); tid++)
  {
    const auto & restartable_data = restartable_datas[tid];

    readRestartableData(restartable_data, recoverable_data, tid);
  }
}
void

RestartableDataIO::readRestartableData(const RestartableDataMap & restartable_data,
                                       const DataNames & recoverable_data,
                                       unsigned int tid)
{
  if (!_in_file_handles[tid].get() || !_in_file_handles[tid]->is_open())
    mooseError("In RestartableDataIO: Need to call readRestartableDataHeader() before calling "
               "readRestartableData()");

  deserializeRestartableData(restartable_data, *_in_file_handles[tid], recoverable_data);

  _in_file_handles[tid]->close();
}

std::shared_ptr<Backup>
RestartableDataIO::createBackup()
{
  std::shared_ptr<Backup> backup = std::make_shared<Backup>();

  serializeSystems(backup->_system_data);

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
  backup->_system_data.seekg(0);
  for (unsigned int tid = 0; tid < n_threads; tid++)
    backup->_restartable_data[tid]->seekg(0);

  deserializeSystems(backup->_system_data);

  const auto & restartable_data_maps = _moose_app.getRestartableData();

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
RestartableDataIO::useAsciiExtension()
{
  _use_binary_ext = false;
}

void
RestartableDataIO::restartEquationSystemsObject()
{
  TIME_SECTION("restartEquationSystems", 3, "Restarting EquationSystems");

  if (!_is_header_read)
    mooseError("In RestartableDataIO: Need to call readRestartableDataHeader() before calling "
               "restartEquationSystemsObject()");

  std::string file_name(_moose_app.getRestartRecoverFileBase() +
                        (_use_binary_ext ? ES_BINARY_EXT : ES_ASCII_EXT));
  MooseUtils::checkFileReadable(file_name);

  unsigned int read_flags = EquationSystems::READ_DATA;
  if (!_fe_problem_ptr->skipAdditionalRestartData())
    read_flags |= EquationSystems::READ_ADDITIONAL_DATA;

  // Set libHilbert renumbering flag to false.  We don't support
  // N-to-M restarts regardless, and if we're *never* going to do
  // N-to-M restarts then libHilbert is just unnecessary computation
  // and communication.
  const bool renumber = false;

  // DECODE or READ based on suffix.
  // MOOSE doesn't currently use partition-agnostic renumbering, since
  // it can break restarts when multiple nodes are at the same point
  _fe_problem_ptr->es().read(file_name, read_flags, renumber);

  for (const auto & nl_sys_num : make_range(_fe_problem_ptr->numNonlinearSystems()))
    _fe_problem_ptr->getNonlinearSystemBase(nl_sys_num).update();
}

void
RestartableDataIO::setErrorOnLoadWithDifferentNumberOfProcessors(bool value)
{
  _error_on_different_number_of_processors = value;
}

void
RestartableDataIO::setErrorOnLoadWithDifferentNumberOfThreads(bool value)
{
  _error_on_different_number_of_threads = value;
}
