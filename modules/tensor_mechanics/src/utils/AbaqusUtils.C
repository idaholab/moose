//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUtils.h"

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

__attribute__((visibility("default"))) extern "C" void
getnumcpus_(int * num)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *num = communicator->size();
}

// Threads

__attribute__((visibility("default"))) extern "C" void
getrank_(int * rank)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *rank = communicator->rank();
}

__attribute__((visibility("default"))) extern "C" int
getnumthreads_()
{
#if defined(LIBMESH_HAVE_TBB_API) ||                                                               \
    (!defined(LIBMESH_HAVE_OPENMP) && defined(LIBMESH_HAVE_PTHREAD))
  return libMesh::n_threads();
#else
  return 1;
#endif
}

__attribute__((visibility("default"))) extern "C" int
get_thread_id_()
{
  ParallelUniqueId puid;
  return puid.id;
}

// Output directory

std::string AbaqusUtils::_output_dir = "";

void
AbaqusUtils::setOutputDir(const std::string & output_dir)
{
  if (!output_dir.empty() && output_dir != _output_dir)
    mooseWarning("Conflicting output directories specified in Abaqus compatibility objects: ",
                 output_dir,
                 " != ",
                 _output_dir,
                 ". Are you running a multiapps simulation?");

  _output_dir = output_dir;
}

__attribute__((visibility("default"))) extern "C" void
getoutdir_(char * dir, unsigned int * len)
{
  auto output_dir = AbaqusUtils::getOutputDir();
  *len = output_dir.length();
  for (unsigned int i = 0; i < 256; ++i)
    dir[i] = i < *len ? output_dir[i] : ' ';
}

// error/warning/info message output

__attribute__((visibility("default"))) extern "C" void
stdb_abqerr_(
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
      mooseInfo(message);
      break;

    case -1:
      mooseWarning(message);
      break;

    case -2:
      mooseWarning("Non fatal Abaqus subroutine error: ", message);
      break;

    case -3:
      mooseError(message);
      break;

    default:
      mooseError("Invalid LOP code passed to STDB_ABQERR: ", *lop);
      break;
  }
}
