//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MemoryUsage.h"

#include <array>
#include <unistd.h>
#include <mpi.h>
#include <fstream>

#ifdef __APPLE__
#include <mach/task.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#else
#include <sys/sysinfo.h>
#endif

registerMooseObject("MooseApp", MemoryUsage);

template <>
InputParameters
validParams<MemoryUsage>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription("Memory usage statistics for the running simulation.");
  MooseEnum mem_type("virtual_memory physical_memory page_faults", "virtual_memory");
  params.addParam<MooseEnum>("mem_type", mem_type, "Memory metric to report.");
  MooseEnum value_type("total average max_process min_process", "total");
  params.addParam<MooseEnum>(
      "value_type", value_type, "Aggregation method to apply to the requested memory metric.");
  params.addParam<bool>("report_peak_value",
                        true,
                        "If the postprocessor is executed more than one "
                        "during a time step, report the aggregated peak "
                        "value.");
  return params;
}

MemoryUsage::MemoryUsage(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mem_type(getParam<MooseEnum>("mem_type").getEnum<MemType>()),
    _value_type(getParam<MooseEnum>("value_type").getEnum<ValueType>()),
    _value(0.0),
    _peak_value(0.0),
    _report_peak_value(getParam<bool>("report_peak_value")),
    _memory_total(0)
{
  // get processor names and assign a unique number to each piece of hardware
  int mpi_namelen;
  char mpi_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(mpi_name, &mpi_namelen);
  std::string processor_name(mpi_name);

  // gather all names at processor zero
  std::vector<std::string> processor_names;
  _communicator.gather(0, processor_name, processor_names);

  // assign a unique numerical id to them on processor zero
  unsigned int id = 0;
  if (processor_id() == 0)
  {
    // map to assign an id to each processor name string
    std::map<std::string, unsigned int> hardware_id_map;
    auto nproc = processor_names.size();
    _hardware_id.resize(nproc);
    for (std::size_t i = 0; i < nproc; ++i)
    {
      auto it = hardware_id_map.lower_bound(processor_names[i]);
      if (it == hardware_id_map.end() || it->first != processor_names[i])
        it = hardware_id_map.emplace_hint(it, processor_names[i], id++);
      _hardware_id[i] = it->second;
    }
    std::cout << "Running on " << id << " nodes\n";
  }

  for (auto id : _hardware_id)
    std::cout << "Hardware id " << id << '\n';

    // get available memory
#ifdef __APPLE__
  uint64_t hwmem_size;
  size_t length = sizeof(hwmem_size);
  if (0 > sysctlbyname("hw.memsize", &hwmem_size, &length, NULL, 0))
    mooseWarning("Unable to query hardware memory size ", name());
  _memory_total = hwmem_size;
#else
  struct sysinfo si_data;
  if (sysinfo(&si_data))
    mooseWarning("Unable to obtain system memory data in ", name());
  _memory_total = si_data.totalram * si_data.mem_unit;
#endif

  std::cout << "Total RAM " << _memory_total << '\n';
}

void
MemoryUsage::timestepSetup()
{
  _peak_value = 0.0;
}

void
MemoryUsage::initialize()
{
  _value = 0.0;
}

void
MemoryUsage::execute()
{
  // data entries are numbered according to their position in /proc/self/stat
  enum StatItem
  {
    index_page_faults = 8,
    index_virtual_size = 19,
    index_resident_size = 20,
    num = 21 // total number of entries read
  };

  // inspect /proc
  std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);
  std::array<unsigned long, 21> val;
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
      val[index_virtual_size] = t_info.virtual_size;
      val[index_resident_size] = t_info.resident_size;
    }
#endif
  }

  // get the requested per core metric
  switch (_mem_type)
  {
    case MemType::virtual_memory:
      _value = val[index_virtual_size];
      break;

    case MemType::physical_memory:
      _value = val[index_resident_size];
      break;

    case MemType::page_faults:
      // major page faults are currently only reported on Linux systems
      _value = val[index_page_faults];
      break;
  }
}

void
MemoryUsage::finalize()
{
  // perform the requested reduction
  switch (_value_type)
  {
    case ValueType::total:
      gatherSum(_value);
      break;

    case ValueType::average:
      gatherSum(_value);
      _value /= n_processors();
      break;

    case ValueType::max_process:
      gatherMax(_value);
      break;

    case ValueType::min_process:
      gatherMin(_value);
      break;

    default:
      mooseError("Invalid value_type");
  }

  if (_value > _peak_value)
    _peak_value = _value;
}

PostprocessorValue
MemoryUsage::getValue()
{
  return _report_peak_value ? _peak_value : _value;
}
