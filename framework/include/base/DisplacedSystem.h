#ifndef DISPLACEDSYSTEM_H_
#define DISPLACEDSYSTEM_H_

#include "SystemBase.h"
// libMesh include
#include "explicit_system.h"
#include "transient_system.h"

class DisplacedProblem;


class DisplacedSystem : public SystemTempl<TransientExplicitSystem>
{
public:
  DisplacedSystem(DisplacedProblem & problem, SystemBase & undisplaced_system, const std::string & name);
  virtual ~DisplacedSystem();

  virtual void prepare(THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);

  virtual NumericVector<Number> & getVector(std::string name);
  
  virtual NumericVector<Number> & serializedSolution() { return _undisplaced_system.serializedSolution(); }

  virtual const NumericVector<Number> * & currentSolution() { return _undisplaced_system.currentSolution(); }

  /// Return the residual copy from the NonlinearSystem
  virtual NumericVector<Number> & residualCopy() { return _undisplaced_system.residualCopy(); }

  /// Return whether or not the NonlinearSystem is currently computing a Jacobian matrix
  virtual bool currentlyComputingJacobian() { return _undisplaced_system.currentlyComputingJacobian(); }
  
protected:
  SystemBase & _undisplaced_system;
};

#endif /* DISPLACEDSYSTEM_H_ */
