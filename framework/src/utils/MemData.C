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

#include "MooseError.h"
#include "MemData.h"

MemData::MemData() :
    _mem_in_kB_at_start(0),
    _mem_in_kB_at_stop(0),
    _started(false)
{
}



MemData::~MemData()
{
}



void MemData::start()
{
  if (_started)
    mooseError("Error! Memory Logging is already active, cannot start() again!");

  this->get_current_mem_usage(_mem_in_kB_at_start, _r_usage_at_start);

  _started = true;
}




void MemData::stop()
{
  if (!_started)
    mooseError("Error! Memory Logging has not yet started stop() now!");

  this->get_current_mem_usage(_mem_in_kB_at_stop, _r_usage_at_stop);

  _started = false;
}




long MemData::delta()
{
  // Don't try to record data in the middle of an event!
  if (_started)
    mooseError("Error! Cannot compute memory usage delta in the middle of an event!");

  return _mem_in_kB_at_stop - _mem_in_kB_at_start;
}




void MemData::get_current_mem_usage(long& mem_in_kB, struct rusage& r_usage)
{
#if (__APPLE__ && __MACH__) // compiler predefines for OSX, http://predef.sourceforge.net/preos.html
  int result = getrusage(RUSAGE_SELF, &r_usage);

  if (result != 0)
    mooseError("Error collecting rusage information!");

  // Store entry(ies) we are interested in.  On the Macs I've used, this value was always reported
  // in bytes, so we divide by 1024
  mem_in_kB = r_usage.ru_maxrss / 1024;

#elif __linux // compiler predefines for Linux, http://predef.sourceforge.net/preos.html

  pid_t pid = getpid();

  // Open the status file
  std::ostringstream oss;
  oss << "/proc/" << pid << "/status";
  std::ifstream file(oss.str().c_str());


  // Process file line by line
  std::string line;
  while (true)
  {
    std::getline(file, line);

    // Check stream. If OK, process line.  We do it this
    // way because std::getline may have set EOF.
    if (file)
    {
      // Search line for "VmSize"
      size_t start = line.find("VmSize");

      // If the substring was found, tokenize the line...
      if (start != std::string::npos)
      {
        std::istringstream iss(line);
        std::string key, units;
        iss >> key >> mem_in_kB >> units;

        // Sanity check, I've never seen a Linux that didn't report values in kB
        if (units != "kB")
          mooseError("VmSize reported in unknown units, cannot continue!");

        // This is the only line we wanted, so break out
        break;
      }

      // This was not the line we were after, so continue to next line
      continue;
    }

    // Check for eof
    if (file.eof())
      break;

    // !file AND !file.eof, the stream is "bad"
    mooseError("Error opening PID status file!");
  }

#else

  mooseWarning("Warning, Unsupported OS, memory logger will return 0.");

#endif
}
