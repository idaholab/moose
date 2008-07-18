//libMesh includes
#include "perf_log.h"

#ifndef STROMA_H
#define STROMA_H

namespace Stroma
{
  /**
   * Perflog to be used by applications.
   * If the application prints this in the end they will get performance info.
   */
  extern PerfLog perf_log;

  /**
   * Registers the Kernels and BCs provided by Stroma.
   */
  void registerObjects();
}

#endif //STROMA_H
