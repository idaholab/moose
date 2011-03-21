#ifndef COMPUTEDAMPINGTHREAD_H
#define COMPUTEDAMPINGTHREAD_H

#include "ThreadedElementLoop.h"
// libMesh includes
#include "elem_range.h"

class NonlinearSystem;


class ComputeDampingThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeDampingThread(Problem & problem, NonlinearSystem & sys, const NumericVector<Number> & update);

  // Splitting Constructor
  ComputeDampingThread(ComputeDampingThread & x, Threads::split split);

  virtual void preElement(const Elem *elem);
  virtual void onElement(const Elem *elem);

  void join(const ComputeDampingThread & /*y*/);

  Real damping() { return _damping; }

protected:
  Real _damping;
  const NumericVector<Number> & _update;
  NonlinearSystem & _nl;
};

#endif //COMPUTEDAMPINGTHREAD_H
