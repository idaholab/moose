//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MemoryUtils.h"
#include "MooseError.h"

#include <unistd.h>
#include <mpi.h>
#include <fstream>
#include <array>

#ifdef __APPLE__
#include <mach/task.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/vm_page_size.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#else
#include <sys/sysinfo.h>
#endif

namespace MemoryUtils
{

std::string
getMPIProcessorName()
{
#ifdef LIBMESH_HAVE_MPI
  int mpi_namelen;
  char mpi_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(mpi_name, &mpi_namelen);
  return mpi_name;
#else
  return "serial";
#endif
}

MooseEnum
getMemUnitsEnum()
{
  return MooseEnum("bytes kibibytes mebibytes gibibytes kilobytes megabytes gigabytes",
                   "mebibytes");
}

std::size_t
getTotalRAM()
{
#ifdef __APPLE__
  uint64_t hwmem_size;
  size_t length = sizeof(hwmem_size);
  if (0 <= sysctlbyname("hw.memsize", &hwmem_size, &length, NULL, 0))
    return hwmem_size;
#else
  struct sysinfo si_data;
  if (!sysinfo(&si_data))
    return si_data.totalram * si_data.mem_unit;
#endif
  return 0;
}

void
getMemoryStats(Stats & stats)
{
  enum StatItem
  {
    index_page_faults = 8,
    index_virtual_size = 19,
    index_resident_size = 20,
    num = 21 // total number of entries read
  };

  // inspect /proc
  std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);
  std::array<std::size_t, 21> val;
  if (stat_stream)
  {
    // if the proc filesystem file is found (Linux) read its contents
    std::string pid, comm, state;
    stat_stream >> pid >> comm >> state;
    for (unsigned int i = 0; i < num; ++i)
      stat_stream >> val[i];

    // resident size is reported as number of pages in /proc
    val[index_resident_size] *= sysconf(_SC_PAGE_SIZE);
  }
  else
  {
    // set all data entries to zero (if all else should fail)
    val.fill(0);

// obtain mach task info on mac OS
#ifdef __APPLE__
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (KERN_SUCCESS == task_info(mach_task_self(),
                                  TASK_BASIC_INFO,
                                  reinterpret_cast<task_info_t>(&t_info),
                                  &t_info_count))
    {
      val[index_virtual_size] = t_info.virtual_size;   // in bytes
      val[index_resident_size] = t_info.resident_size; // in bytes
    }
    else
      mooseDoOnce(::mooseWarning("task_info call failed, memory usage numbers will be incorrect"));
#endif
  }

  // physical mem
  stats._physical_memory = val[index_resident_size];

  // virtual mem
  stats._virtual_memory = val[index_virtual_size];

  // page faults
  stats._page_faults = val[index_page_faults];
}

std::size_t
convertBytes(std::size_t bytes, MemUnits unit)
{
  if (unit == MemUnits::Bytes)
    return bytes;

  unsigned int nunit = static_cast<unsigned int>(unit);

  // kibi, mebi, gibi
  if (nunit <= 3)
    return bytes >> (nunit * 10);

  // kilo, mega, giga
  if (nunit <= 6)
  {
    while (nunit-- > 3)
      bytes /= 1000;
    return bytes;
  }

  mooseError("Unknown memory unit");
}

} // namespace MemoryUtils
