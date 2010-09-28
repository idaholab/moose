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

//libMesh includes
#include "libmesh_common.h"
#include "ValidParams.h"

//Forward Declarations
class Postprocessor;
class MooseSystem;

template<>
InputParameters validParams<Postprocessor>();

class Postprocessor
{
public:
  Postprocessor(const std::string & name, MooseSystem &moose_system, InputParameters parameters);
  
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
   * returns the name of this object
   */
  virtual const std::string & name();
  
  /**
   * Gather the parallel sum of the variable passed in.
   *
   * After calling this, the variable that was passed in will hold the gathered value.
   */
  void gatherSum(Real & value);

  /**
   * Same but for ints.
   */
  void gatherSum(int & value);

private:
  std::string _local_name;
  THREAD_ID _local_tid;
};
 
#endif
