#ifndef POSTPROCESSOR_H_
#define POSTPROCESSOR_H_

#include <string>

//MOOSE includes
#include "Moose.h"
#include "Object.h"
#include "ParallelUniqueId.h"

//libMesh includes
#include "libmesh_common.h"
#include "parallel.h"
//#include "ValidParams.h"

namespace Moose {
  class Problem;
}

//Forward Declarations
class Postprocessor;


template<>
InputParameters validParams<Postprocessor>();

class Postprocessor :
  public Object
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

//  /**
//   * returns the name of this object
//   */
//  virtual const std::string & name();
  
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
  Moose::Problem & _problem;
  THREAD_ID _tid;
};
 
#endif
