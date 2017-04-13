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

#ifndef COMPUTENODALAUXVARSTHREAD_H
#define COMPUTENODALAUXVARSTHREAD_H

// MOOSE includes
#include "ThreadedNodeLoop.h"

// libMesh includes
#include "libmesh/node_range.h"

// Forward declarations
class AuxiliarySystem;
class AuxKernel;
class FEProblemBase;
template <typename T>
class MooseObjectWarehouse;

class ComputeNodalAuxVarsThread
    : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalAuxVarsThread(FEProblemBase & fe_problem,
                            const MooseObjectWarehouse<AuxKernel> & storage);
  // Splitting Constructor
  ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split split);

  void onNode(ConstNodeRange::const_iterator & nd);

  void join(const ComputeNodalAuxVarsThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernel> & _storage;
};

#endif // COMPUTENODALAUXVARSTHREAD_H
