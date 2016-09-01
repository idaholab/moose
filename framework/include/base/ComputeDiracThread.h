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

#ifndef COMPUTEDIRACTHREAD_H
#define COMPUTEDIRACTHREAD_H

// Moose Includes
#include "ParallelUniqueId.h"
#include "ThreadedElementLoop.h"

// libMesh includes
#include "libmesh/elem_range.h"

// Forward declarations
class NonlinearSystemBase;
class DiracKernel;

typedef StoredRange<std::set<const Elem *>::const_iterator, const Elem *> DistElemRange;


class ComputeDiracThread : public ThreadedElementLoop<DistElemRange>
{
public:
  ComputeDiracThread(FEProblem & feproblem, NonlinearSystemBase & system, SparseMatrix<Number> * jacobian = NULL);

  // Splitting Constructor
  ComputeDiracThread(ComputeDiracThread & x, Threads::split);

  virtual ~ComputeDiracThread();

  virtual void subdomainChanged() override;
  virtual void pre() override;
  virtual void onElement(const Elem * elem) override;
  virtual void postElement(const Elem * /*elem*/) override;
  virtual void post() override;

  void join(const ComputeDiracThread & /*y*/);

protected:
  SparseMatrix<Number> * _jacobian;
  NonlinearSystemBase & _sys;

  /// Storage for DiracKernel objects
  const MooseObjectWarehouse<DiracKernel> & _dirac_kernels;
};

#endif //COMPUTEDIRACTHREAD_H
