#ifndef COMPUTEDIRACTHREAD_H
#define COMPUTEDIRACTHREAD_H

//Moose Includes
#include "ParallelUniqueId.h"
#include "DiracKernelWarehouse.h"
#include "DiracKernel.h"
#include "ThreadedElementLoop.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "elem_range.h"

#include <vector>
typedef StoredRange<std::set<const Elem *>::const_iterator, const Elem *> DistElemRange;
class ComputeDiracThread : public ThreadedElementLoop<DistElemRange>
{
public:
  ComputeDiracThread(Problem & problem, NonlinearSystem & system, NumericVector<Number> * residual, SparseMatrix<Number> * jacobian = NULL);

  // Splitting Constructor
  ComputeDiracThread(ComputeDiracThread & x, Threads::split);

  virtual void preElement(const Elem * /*elem*/);
  virtual void onElement(const Elem *elem);
  virtual void postElement(const Elem * /*elem*/);

  void join(const ComputeDiracThread & /*y*/);

protected:
  NumericVector<Number> * _residual;
  SparseMatrix<Number> * _jacobian;
  NonlinearSystem & _sys;
  std::set<MooseVariable *> _vars;
};

#endif //COMPUTEDIRACTHREAD_H
