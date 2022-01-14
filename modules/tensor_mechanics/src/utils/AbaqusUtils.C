//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUtils.h"
#include "MooseUtils.h"
#include "libmesh/threads.h"

// MPI

const libMesh::Parallel::Communicator * AbaqusUtils::_communicator = nullptr;

void
AbaqusUtils::setCommunicator(const libMesh::Parallel::Communicator * communicator)
{
  if (_communicator != nullptr && communicator != _communicator)
    mooseWarning("Conflicting MPI communicators specified in Abaqus compatibility objects. Are you "
                 "running a multiapps simulation?");

  _communicator = communicator;
}

extern "C" void
getnumcpus_(int * num)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *num = communicator->size();
}

extern "C" void
getrank_(int * rank)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *rank = communicator->rank();
}

extern "C" MPI_Comm
get_communicator()
{
  auto communicator = AbaqusUtils::getCommunicator();
  return communicator->get();
}

// Threads

extern "C" int
getnumthreads_()
{
  return libMesh::n_threads();
}

extern "C" int
get_thread_id_()
{
  ParallelUniqueId puid;
  return puid.id;
}

// Output directory

std::string AbaqusUtils::_output_dir = "";
std::string AbaqusUtils::_job_name = "";

void
AbaqusUtils::setInputFile(const std::string & input_file)
{
  auto split = MooseUtils::splitFileName(input_file);
  auto output_dir = split.first;
  auto job_name = MooseUtils::stripExtension(split.second);

  if (!_output_dir.empty() && output_dir != _output_dir)
    mooseWarning("Conflicting output directories specified in Abaqus compatibility objects: ",
                 output_dir,
                 " != ",
                 _output_dir,
                 ". Are you running a multiapps simulation?");

  if (!_job_name.empty() && job_name != _job_name)
    mooseWarning("Conflicting job names specified in Abaqus compatibility objects: ",
                 job_name,
                 " != ",
                 _job_name,
                 ". Are you running a multiapps simulation?");

  _output_dir = output_dir;
  _job_name = job_name;
}

extern "C" void
getoutdir_(char * dir, int * len)
{
  auto output_dir = AbaqusUtils::getOutputDir();
  *len = output_dir.length();
  for (int i = 0; i < 256; ++i)
    dir[i] = i < *len ? output_dir[i] : ' ';
}

extern "C" void
getjobname_(char * dir, int * len)
{
  auto job_name = AbaqusUtils::getJobName();
  *len = job_name.length();
  for (int i = 0; i < 256; ++i)
    dir[i] = i < *len ? job_name[i] : ' ';
}

// error/warning/info message output

extern "C" void
stdb_abqerr_(int * lop, char * format, int * intv, double * realv, char * charv, int format_len)
{
  std::string message;
  unsigned int int_index = 0;
  unsigned int real_index = 0;
  unsigned int char_index = 0;

  for (int i = 0; i < format_len; ++i)
  {
    // interpret %I, %R, and %S
    if (format[i] == '%' && i < format_len - 1)
    {
      auto next = format[i + 1];

      // integer output
      if (next == 'I' || next == 'i')
      {
        message += std::to_string(intv[int_index++]);
        i++;
        continue;
      }

      // Real output
      if (next == 'R' || next == 'r')
      {
        message += std::to_string(realv[real_index++]);
        i++;
        continue;
      }

      // char[8] output
      if (next == 'S' || next == 's')
      {
        for (unsigned int j = 0; j < 8; ++j)
          message += charv[char_index++];
        i++;
        continue;
      }
    }

    // append character to string
    message += format[i];
  }

  // output at the selected error level
  switch (*lop)
  {
    case 1:
      Moose::out << moose::internal::mooseMsgFmt(message, "** Abaqus Info **", COLOR_CYAN)
                 << std::flush;
      break;

    case -1:
      Moose::out << moose::internal::mooseMsgFmt(message, "** Abaqus Warning **", COLOR_YELLOW)
                 << std::flush;
      break;

    case -2:
      Moose::out << moose::internal::mooseMsgFmt(message, "** Abaqus Non-fatal Error **", COLOR_RED)
                 << std::flush;
      break;

    case -3:
      mooseError(message);
      break;

    default:
      mooseError("Invalid LOP code passed to STDB_ABQERR: ", *lop);
      break;
  }
}

void
AbaqusUtils::smaInitialize()
{
  static bool initialized = false;

  // Guard the initialization with a double checked lock
  if (!initialized)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    if (!initialized)
    {
      const auto n = getnumthreads_();
      _sma_local_int_array.resize(n);
      _sma_local_float_array.resize(n);
      initialized = true;
    }
  }
}

// Array creation

std::map<int, std::vector<int>> AbaqusUtils::_sma_int_array;
std::map<int, std::vector<Real>> AbaqusUtils::_sma_float_array;
std::vector<std::map<int, std::vector<int>>> AbaqusUtils::_sma_local_int_array;
std::vector<std::map<int, std::vector<Real>>> AbaqusUtils::_sma_local_float_array;

extern "C" int *
SMAIntArrayCreate(int id, int len, int val)
{
  auto ib = AbaqusUtils::_sma_int_array.emplace(id, std::vector<int>(len, val));
  if (ib.second == false)
    mooseError("Error creating threaded storage in SMAIntArrayCreate");
  return ib.first->second.data();
}

extern "C" double *
SMAFloatArrayCreate(int id, int len, Real val)
{
  auto ib = AbaqusUtils::_sma_float_array.emplace(id, std::vector<Real>(len, val));
  if (ib.second == false)
    mooseError("Error creating threaded storage in SMAFloatArrayCreate");
  return ib.first->second.data();
}

extern "C" int *
SMALocalIntArrayCreate(int id, int len, int val)
{
  AbaqusUtils::smaInitialize();
  auto & array = AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_int_array,
                                                "SMALocalIntArrayCreate")[id];
  array.assign(len, val);
  return array.data();
}

extern "C" double *
SMALocalFloatArrayCreate(int id, int len, Real val)
{
  AbaqusUtils::smaInitialize();
  auto & array = AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_float_array,
                                                "SMALocalFloatArrayCreate")[id];
  array.assign(len, val);
  return array.data();
}

// Array access

extern "C" int *
SMAIntArrayAccess(int id)
{
  auto it = AbaqusUtils::getSMAIterator(AbaqusUtils::_sma_int_array, id, "SMAIntArrayAccess");
  return it->second.data();
}

extern "C" double *
SMAFloatArrayAccess(int id)
{
  auto it = AbaqusUtils::getSMAIterator(AbaqusUtils::_sma_float_array, id, "SMAFloatArrayAccess");
  return it->second.data();
}

extern "C" int *
SMALocalIntArrayAccess(int id)
{
  auto & array =
      AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_int_array, "SMALocalIntArrayAccess");
  auto it = AbaqusUtils::getSMAIterator(array, id, "SMALocalIntArrayAccess");
  return it->second.data();
}

extern "C" double *
SMALocalFloatArrayAccess(int id)
{
  auto & array = AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_float_array,
                                                "SMALocalFloatArrayAccess");
  auto it = AbaqusUtils::getSMAIterator(array, id, "SMALocalFloatArrayAccess");
  return it->second.data();
}

// Array size check

extern "C" std::size_t
SMAIntArraySize(int id)
{
  auto it = AbaqusUtils::getSMAIterator(AbaqusUtils::_sma_int_array, id, "SMAIntArraySize");
  return it->second.size();
}

extern "C" std::size_t
SMAFloatArraySize(int id)
{
  auto it = AbaqusUtils::getSMAIterator(AbaqusUtils::_sma_float_array, id, "SMAFloatArraySize");
  return it->second.size();
}

extern "C" std::size_t
SMALocalIntArraySize(int id)
{
  auto & array =
      AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_int_array, "SMALocalIntArraySize");
  auto it = AbaqusUtils::getSMAIterator(array, id, "SMALocalIntArraySize");
  return it->second.size();
}

extern "C" std::size_t
SMALocalFloatArraySize(int id)
{
  auto & array =
      AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_float_array, "SMALocalFloatArraySize");
  auto it = AbaqusUtils::getSMAIterator(array, id, "SMALocalFloatArraySize");
  return it->second.size();
}

// Array deletion

extern "C" void
SMAIntArrayDelete(int id)
{
  auto it = AbaqusUtils::getSMAIterator(AbaqusUtils::_sma_int_array, id, "SMAIntArrayDelete");
  AbaqusUtils::_sma_int_array.erase(it);
}

extern "C" void
SMAFloatArrayDelete(int id)
{
  auto it = AbaqusUtils::getSMAIterator(AbaqusUtils::_sma_float_array, id, "SMAFloatArrayDelete");
  AbaqusUtils::_sma_float_array.erase(it);
}

extern "C" void
SMALocalIntArrayDelete(int id)
{
  auto & array =
      AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_int_array, "SMALocalIntArrayDelete");
  auto it = AbaqusUtils::getSMAIterator(array, id, "SMALocalIntArrayDelete");
  array.erase(it);
}

extern "C" void
SMALocalFloatArrayDelete(int id)
{
  auto & array = AbaqusUtils::getSMAThreadArray(AbaqusUtils::_sma_local_float_array,
                                                "SMALocalFloatArrayDelete");
  auto it = AbaqusUtils::getSMAIterator(array, id, "SMALocalFloatArrayDelete");
  array.erase(it);
}

// Mutex handling

std::array<std::unique_ptr<Threads::spin_mutex>, 101> AbaqusUtils::_mutex = {{nullptr}};

void
AbaqusUtils::mutexInit(std::size_t n)
{
  // Guard the initialization with a double checked lock
  if (!_mutex[n])
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    if (!_mutex[n])
      _mutex[n] = std::make_unique<Threads::spin_mutex>();
  }
}

void
AbaqusUtils::mutexLock(std::size_t n)
{
  if (n >= _mutex.size() || !_mutex[n])
    mooseError("Invalid or uninitialized mutex ", n);
  _mutex[n]->lock();
}

void
AbaqusUtils::mutexUnlock(std::size_t n)
{
  if (n >= _mutex.size() || !_mutex[n])
    mooseError("Invalid or uninitialized mutex ", n);
  _mutex[n]->unlock();
}

extern "C" void
MutexInit(int id)
{
  AbaqusUtils::mutexInit(id);
}

extern "C" void
MutexLock(int id)
{
  AbaqusUtils::mutexLock(id);
}

extern "C" void
MutexUnlock(int id)
{
  AbaqusUtils::mutexUnlock(id);
}
