#ifndef COMPUTERESIDUALTHREAD_H
#define COMPUTERESIDUALTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "elem_range.h"

namespace Moose
{

class ComputeResidualThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeResidualThread(Problem & problem, NonlinearSystem & sys, NumericVector<Number> & residual);  

  // Splitting Constructor
  ComputeResidualThread(ComputeResidualThread & x, Threads::split split);

  virtual void preElement(const Elem *elem);
  virtual void onElement(const Elem *elem);
  virtual void onDomainChanged(short int subdomain);
  virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id);
  virtual void postElement(const Elem * /*elem*/);

  void join(const ComputeResidualThread & /*y*/);

protected:
  NumericVector<Number> & _residual;
  NonlinearSystem & _sys;
  std::set<Variable *> _vars;
};
  
}

#endif //COMPUTERESIDUALTHREAD_H
