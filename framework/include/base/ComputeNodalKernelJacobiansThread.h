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

#ifndef COMPUTENODALKERNELJACOBIANSTHREAD_H
#define COMPUTENODALKERNELJACOBIANSTHREAD_H

#include "ThreadedNodeLoop.h"
#include "NodalKernelWarehouse.h"

// libMesh includes
#include "libmesh/node_range.h"

// Forward declarations
class FEProblem;
class AuxiliarySystem;

// libMesh forward declarations
namespace libMesh
{
template <typename T> class SparseMatrix;
}


class ComputeNodalKernelJacobiansThread : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalKernelJacobiansThread(FEProblem & fe_problem,
                                    AuxiliarySystem & sys,
                                    std::vector<NodalKernelWarehouse> & nodal_kernels,
                                    SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeNodalKernelJacobiansThread(ComputeNodalKernelJacobiansThread & x, Threads::split split);

  virtual void pre();

  virtual void onNode(ConstNodeRange::const_iterator & node_it);

  void join(const ComputeNodalKernelJacobiansThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  std::vector<NodalKernelWarehouse> & _nodal_kernels;

  SparseMatrix<Number> & _jacobian;

  /// Number of contributions cached up
  unsigned int _num_cached;
};

#endif //COMPUTENODALKERNELJACOBIANSTHREAD_H
