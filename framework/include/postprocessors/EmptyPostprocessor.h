#ifndef EMPTYPOSTPROCESSOR_H
#define EMPTYPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class EmptyPostprocessor;

template<>
InputParameters validParams<EmptyPostprocessor>();

class EmptyPostprocessor : public GeneralPostprocessor
{
public:
  EmptyPostprocessor(const std::string & name, InputParameters parameters);
  
  virtual ~EmptyPostprocessor(){ }
  
  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize(){};
  
  /**
   * This function will get called on each geometric object this EmptyPostprocessor acts on
   * (ie Elements, Sides or Nodes).  This will most likely get called multiple times
   * before getValue() is called.
   *
   * Someone somewhere has to override this.
   */
  virtual void execute(){};

  /**
   * This will get called to actually grab the final value the EmptyPostprocessor has calculated.
   */
  virtual Real getValue() { return 0; }
};
 
#endif
