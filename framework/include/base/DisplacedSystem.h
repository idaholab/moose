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

#ifndef DISPLACEDSYSTEM_H
#define DISPLACEDSYSTEM_H

#include "Moose.h"
#include "SystemBase.h"
// libMesh include
#include "explicit_system.h"
#include "transient_system.h"

class DisplacedProblem;


class DisplacedSystem : public SystemTempl<TransientExplicitSystem>
{
public:
  DisplacedSystem(DisplacedProblem & problem, SystemBase & undisplaced_system, const std::string & name, Moose::VarKindType var_kind);
  virtual ~DisplacedSystem();

  virtual void init();

  virtual NumericVector<Number> & getVector(std::string name);

  virtual NumericVector<Number> & serializedSolution() { return _undisplaced_system.serializedSolution(); }

  virtual const NumericVector<Number> * & currentSolution() { return _undisplaced_system.currentSolution(); }

  /// Return the residual copy from the NonlinearSystem
  virtual NumericVector<Number> & residualCopy() { return _undisplaced_system.residualCopy(); }
  virtual NumericVector<Number> & residualGhosted() { return _undisplaced_system.residualGhosted(); }

  virtual void augmentSendList(std::vector<unsigned int> & send_list){ _undisplaced_system.augmentSendList(send_list); }

  // This is an empty function since the displaced system doesn't have a matrix!
  // All sparsity pattern modification will be taken care of by the undisplaced system directly
  virtual void augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                               std::vector<unsigned int> & /*n_nz*/,
                               std::vector<unsigned int> & /*n_oz*/)
    {}

  /// Return whether or not the NonlinearSystem is currently computing a Jacobian matrix
  virtual bool currentlyComputingJacobian() { return _undisplaced_system.currentlyComputingJacobian(); }

protected:
  SystemBase & _undisplaced_system;
};

#endif /* DISPLACEDSYSTEM_H */
