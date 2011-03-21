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

class Problem;
class SubProblemInterface;

//Forward Declarations
class Postprocessor;


template<>
InputParameters validParams<Postprocessor>();

class Postprocessor :
  public MooseObject
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
   * Gather the parallel sum of the variable passed in.
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  template <typename T>
  void gatherSum(T & value)
  {
    // TODO: Gather threaded values as well
    Parallel::sum(value);
  }

protected:
  Problem & _problem;
  SubProblemInterface & _subproblem;
  THREAD_ID _tid;
};
 
#endif
