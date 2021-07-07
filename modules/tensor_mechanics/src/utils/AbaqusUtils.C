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

extern "C" void __attribute__((visibility("default"))) getnumcpus_(int * num)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *num = communicator->size();
}

extern "C" TIMPI::communicator __attribute__((visibility("default"))) get_communicator()
{
  auto communicator = AbaqusUtils::getCommunicator();
  return communicator->get();
}

// Threads

extern "C" void __attribute__((visibility("default"))) getrank_(int * rank)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *rank = communicator->rank();
}

extern "C" int __attribute__((visibility("default"))) getnumthreads_()
{
#if defined(LIBMESH_HAVE_TBB_API) ||                                                               \
    (!defined(LIBMESH_HAVE_OPENMP) && defined(LIBMESH_HAVE_PTHREAD))
  return libMesh::n_threads();
#else
  return 1;
#endif
}

extern "C" int __attribute__((visibility("default"))) get_thread_id_()
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

extern "C" void __attribute__((visibility("default"))) getoutdir_(char * dir, unsigned int * len)
{
  auto output_dir = AbaqusUtils::getOutputDir();
  *len = output_dir.length();
  for (unsigned int i = 0; i < 256; ++i)
    dir[i] = i < *len ? output_dir[i] : ' ';
}

extern "C" void __attribute__((visibility("default"))) getjobname_(char * dir, unsigned int * len)
{
  auto job_name = AbaqusUtils::getJobName();
  *len = job_name.length();
  for (unsigned int i = 0; i < 256; ++i)
    dir[i] = i < *len ? job_name[i] : ' ';
}

// error/warning/info message output

extern "C" void __attribute__((visibility("default"))) stdb_abqerr_(
    int * lop, char * format, int * intv, double * realv, char * charv, std::size_t format_len)
{
  std::string message;
  unsigned int int_index = 0;
  unsigned int real_index = 0;
  unsigned int char_index = 0;

  for (unsigned int i = 0; i < format_len; ++i)
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
      auto n = getnumthreads_();
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

extern "C" int * __attribute__((visibility("default"))) SMAIntArrayCreate(int id, int len, int val)
{
  AbaqusUtils::_sma_int_array[id].assign(len, val);
  return AbaqusUtils::_sma_int_array[id].data();
}

extern "C" Real * __attribute__((visibility("default")))
SMAFloatArrayCreate(int id, int len, Real val)
{
  AbaqusUtils::_sma_float_array[id].assign(len, val);
  return AbaqusUtils::_sma_float_array[id].data();
}

extern "C" int * __attribute__((visibility("default")))
SMALocalIntArrayCreate(int id, int len, int val)
{
  ParallelUniqueId puid;
  AbaqusUtils::smaInitialize();
  auto & array = AbaqusUtils::_sma_local_int_array[puid.id][id];
  array.assign(len, val);
  return array.data();
}

extern "C" Real * __attribute__((visibility("default")))
SMALocalFloatArrayCreate(int id, int len, Real val)
{
  ParallelUniqueId puid;
  AbaqusUtils::smaInitialize();
  auto & array = AbaqusUtils::_sma_local_float_array[puid.id][id];
  array.assign(len, val);
  return array.data();
}

// Array access

extern "C" int * __attribute__((visibility("default"))) SMAIntArrayAccess(int id)
{
  auto it = AbaqusUtils::_sma_int_array.find(id);
  if (it == AbaqusUtils::_sma_int_array.end())
    mooseError("Invalid id ", id, " in SMAIntArrayAccess.");
  return it->second.data();
}

extern "C" Real * __attribute__((visibility("default"))) SMAFloatArrayAccess(int id)
{
  auto it = AbaqusUtils::_sma_float_array.find(id);
  if (it == AbaqusUtils::_sma_float_array.end())
    mooseError("Invalid id ", id, " in SMAFloatArrayAccess.");
  return it->second.data();
}

extern "C" int * __attribute__((visibility("default"))) SMALocalIntArrayAccess(int id)
{
  ParallelUniqueId puid;
  mooseAssert(puid.id < AbaqusUtils::_sma_local_int_array.size(),
              "SMA storage not properly initialized");
  auto it = AbaqusUtils::_sma_local_int_array[puid.id].find(id);
  if (it == AbaqusUtils::_sma_local_int_array[puid.id].end())
    mooseError("Invalid id ", id, " in SMALocalIntArrayAccess.");
  return it->second.data();
}

extern "C" Real * __attribute__((visibility("default"))) SMALocalFloatArrayAccess(int id)
{
  ParallelUniqueId puid;
  mooseAssert(puid.id < AbaqusUtils::_sma_local_float_array.size(),
              "SMA storage not properly initialized");
  auto it = AbaqusUtils::_sma_local_float_array[puid.id].find(id);
  if (it == AbaqusUtils::_sma_local_float_array[puid.id].end())
    mooseError("Invalid id ", id, " in SMALocalFloatArrayAccess.");
  return it->second.data();
}

// Array size check

extern "C" std::size_t __attribute__((visibility("default"))) SMAIntArraySize(int id)
{
  auto it = AbaqusUtils::_sma_int_array.find(id);
  if (it == AbaqusUtils::_sma_int_array.end())
    mooseError("Invalid id ", id, " in SMAIntArraySize.");
  return it->second.size();
}

extern "C" std::size_t __attribute__((visibility("default"))) SMAFloatArraySize(int id)
{
  auto it = AbaqusUtils::_sma_float_array.find(id);
  if (it == AbaqusUtils::_sma_float_array.end())
    mooseError("Invalid id ", id, " in SMAFloatArraySize.");
  return it->second.size();
}

extern "C" std::size_t __attribute__((visibility("default"))) SMALocalIntArraySize(int id)
{
  ParallelUniqueId puid;
  mooseAssert(puid.id < AbaqusUtils::_sma_local_int_array.size(),
              "SMA storage not properly initialized");
  auto it = AbaqusUtils::_sma_local_int_array[puid.id].find(id);
  if (it == AbaqusUtils::_sma_local_int_array[puid.id].end())
    mooseError("Invalid id ", id, " in SMALocalIntArraySize.");
  return it->second.size();
}

extern "C" std::size_t __attribute__((visibility("default"))) SMALocalFloatArraySize(int id)
{
  ParallelUniqueId puid;
  mooseAssert(puid.id < AbaqusUtils::_sma_local_float_array.size(),
              "SMA storage not properly initialized");
  auto it = AbaqusUtils::_sma_local_float_array[puid.id].find(id);
  if (it == AbaqusUtils::_sma_local_float_array[puid.id].end())
    mooseError("Invalid id ", id, " in SMALocalFloatArraySize.");
  return it->second.size();
}

// Array deletion

extern "C" void __attribute__((visibility("default"))) SMAIntArrayDelete(int id)
{
  auto it = AbaqusUtils::_sma_int_array.find(id);
  if (it == AbaqusUtils::_sma_int_array.end())
    mooseError("Invalid id ", id, " in SMAIntArrayDelete.");
  AbaqusUtils::_sma_int_array.erase(it);
}

extern "C" void __attribute__((visibility("default"))) SMAFloatArrayDelete(int id)
{
  auto it = AbaqusUtils::_sma_float_array.find(id);
  if (it == AbaqusUtils::_sma_float_array.end())
    mooseError("Invalid id ", id, " in SMAFloatArrayDelete.");
  AbaqusUtils::_sma_float_array.erase(it);
}

extern "C" void __attribute__((visibility("default"))) SMALocalIntArrayDelete(int id)
{
  ParallelUniqueId puid;
  mooseAssert(puid.id < AbaqusUtils::_sma_local_int_array.size(),
              "SMA storage not properly initialized");
  auto it = AbaqusUtils::_sma_local_int_array[puid.id].find(id);
  if (it == AbaqusUtils::_sma_local_int_array[puid.id].end())
    mooseError("Invalid id ", id, " in SMALocalIntArrayDelete.");
  AbaqusUtils::_sma_local_int_array[puid.id].erase(it);
}

extern "C" void __attribute__((visibility("default"))) SMALocalFloatArrayDelete(int id)
{
  ParallelUniqueId puid;
  mooseAssert(puid.id < AbaqusUtils::_sma_local_float_array.size(),
              "SMA storage not properly initialized");
  auto it = AbaqusUtils::_sma_local_float_array[puid.id].find(id);
  if (it == AbaqusUtils::_sma_local_float_array[puid.id].end())
    mooseError("Invalid id ", id, " in SMALocalFloatArrayDelete.");
  AbaqusUtils::_sma_local_float_array[puid.id].erase(it);
}

// Mutex handling

std::array<std::unique_ptr<Threads::spin_mutex>, 101> AbaqusUtils::_mutex = {nullptr};

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

extern "C" void __attribute__((visibility("default"))) MutexInit(int id)
{
  AbaqusUtils::mutexInit(id);
}

extern "C" void __attribute__((visibility("default"))) MutexLock(int id)
{
  AbaqusUtils::mutexLock(id);
}

extern "C" void __attribute__((visibility("default"))) MutexUnlock(int id)
{
  AbaqusUtils::mutexUnlock(id);
}
