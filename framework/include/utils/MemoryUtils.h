//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseEnum.h"

namespace MemoryUtils
{

/// get the MPI hostname
std::string getMPIProcessorName();

struct Stats
{
  std::size_t _physical_memory;
  std::size_t _virtual_memory;
  std::size_t _page_faults;
};

enum class MemUnits
{
  Bytes,
  Kibibytes,
  Mebibytes,
  Gibibytes = 3,
  Kilobytes,
  Megabytes,
  Gigabytes = 6,
};

/// get the moose enum for the mem_unit_prefix parameter
MooseEnum getMemUnitsEnum();

/// get the total amount of physical RAM available
std::size_t getTotalRAM();

/// get all memory stats for the current process
/// stats The Stats object to fill with the data
/// @return true for success, false for failure
bool getMemoryStats(Stats & stats);

/// convert bytes to selected unit prefix
std::size_t convertBytes(std::size_t bytes, MemUnits unit);

} // namespace MemoryUtils
