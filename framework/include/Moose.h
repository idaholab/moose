//libMesh includes
#include "perf_log.h"

#ifndef MOOSE_H
#define MOOSE_H

namespace Moose
{
  /**
   * Perflog to be used by applications.
   * If the application prints this in the end they will get performance info.
   */
  extern PerfLog perf_log;

  /**
   * Registers the Kernels and BCs provided by Moose.
   */
  void registerObjects();
}

#endif //MOOSE_H
