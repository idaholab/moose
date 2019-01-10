//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MEMORYUTILS_H
#define MEMORYUTILS_H

#include "Moose.h"

namespace MemoryUtils
{

/// get the total amount of physical RAM available
std::size_t getTotalRAM();

/// get the MPI hostname
std::string getMPIProcessorName();

struct Stats
{
  std::size_t _physical_memory;
  std::size_t _virtual_memory;
  std::size_t _page_faults;
};

enum class MemUnit
{
  Bytes,
  Kilobytes,
  Megabytes,
  Gigabytes,
};

/// get all memory stats for the current process
void getMemoryStats(Stats & stats, MemUnit units = MemUnit::Megabytes);

} // namespace MemoryUtils

#endif // MEMORYUTILS_H
