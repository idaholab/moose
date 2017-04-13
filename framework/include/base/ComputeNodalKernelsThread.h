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

#ifndef COMPUTENODALKERNELSTHREAD_H
#define COMPUTENODALKERNELSTHREAD_H

#include "ThreadedNodeLoop.h"

// libMesh includes
#include "libmesh/node_range.h"

// Forward declarations
class FEProblemBase;
class AuxiliarySystem;
class NodalKernel;

class ComputeNodalKernelsThread
    : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalKernelsThread(FEProblemBase & fe_problem,
                            const MooseObjectWarehouse<NodalKernel> & nodal_kernels);

  // Splitting Constructor
  ComputeNodalKernelsThread(ComputeNodalKernelsThread & x, Threads::split split);

  virtual void pre() override;

  virtual void onNode(ConstNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalKernelsThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  const MooseObjectWarehouse<NodalKernel> & _nodal_kernels;

  /// Number of contributions cached up
  unsigned int _num_cached;
};

#endif // COMPUTENODALKERNELSTHREAD_H
