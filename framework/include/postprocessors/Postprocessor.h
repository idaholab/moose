#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include "MooseObject.h"

//Forward Declarations
class Postprocessor;

template<>
InputParameters validParams<Postprocessor>();

class Postprocessor : public MooseObject
{
public:
  Postprocessor(std::string name, MooseSystem &moose_system, InputParameters parameters);
  
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
  virtual Real getValue() = 0;

  /**
   * Gather the parallel sum of the variable passed in.
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  void gatherSum(Real value);
};
 
#endif
