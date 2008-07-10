#include "KernelFactory.h"
#include "BCFactory.h"
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "VectorNeumannBC.h"

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
  PerfLog perf_log("Application");

  /**
   * Registers the Kernels and BCs provided by Stroma.
   */
  void registerObjects()
  {
    BCFactory::instance()->registerBC<DirichletBC>("DirichletBC");
    BCFactory::instance()->registerBC<NeumannBC>("NeumannBC");
    BCFactory::instance()->registerBC<VectorNeumannBC>("VectorNeumannBC");
  }
}

#endif //STROMA_H
