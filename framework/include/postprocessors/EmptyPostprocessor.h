/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef EMPTYPOSTPROCESSOR_H
#define EMPTYPOSTPROCESSOR_H

#include "Postprocessor.h"

//Forward Declarations
class EmptyPostprocessor;

template<>
InputParameters validParams<EmptyPostprocessor>();

class EmptyPostprocessor : public Postprocessor
{
public:
  EmptyPostprocessor(std::string name, MooseSystem &moose_system, InputParameters parameters);
  
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
  virtual Real getValue(){ return 0;}
};
 
#endif
