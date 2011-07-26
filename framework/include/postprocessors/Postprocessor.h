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

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <string>

//MOOSE includes
#include "Moose.h"
#include "MooseObject.h"
#include "ParallelUniqueId.h"

//libMesh includes
#include "libmesh_common.h"
#include "parallel.h"

class Postprocessor;
class Problem;
class SubProblemInterface;

template<>
InputParameters validParams<Postprocessor>();


class Postprocessor : public MooseObject
{
public:
  Postprocessor(const std::string & name, InputParameters parameters);
  
  virtual ~Postprocessor(){ }
  
  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() = 0;
  
  /**
   * This function will get called on each geometric object this postprocessor acts on
   * (ie Elements, Sides or Nodes).  This will most likely get called multiple times
   * before getValue() is called.
   *
   * Someone somewhere has to override this.
   */
  virtual void execute() = 0;

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   */
  virtual PostprocessorValue getValue() = 0;

  /**
   * Gather the parallel sum of the variable passed in. It HAS to take care of values across all threads and CPUs (we DO hybrid parallelism!)
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  template <typename T>
  void gatherSum(T & value)
  {
    // TODO: Gather threaded values as well
    Parallel::sum(value);
  }

  virtual void threadJoin(const Postprocessor & /*pps*/) { }

  /**
   * Is the postprocessor marked for output?
   * @return true if it is outputted, otherwise false
   */
  bool getOutput() { return _output; }

protected:
  Problem & _problem;
  SubProblemInterface & _subproblem;
  THREAD_ID _tid;

  bool _output;                 ///< true if the postprocessor value is outputted or not
};

#endif
