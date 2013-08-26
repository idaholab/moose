;/****************************************************************/
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

#ifndef SIDEPOSTPROCESSOR_H
#define SIDEPOSTPROCESSOR_H

#include "SideUserObject.h"
#include "Postprocessor.h"

//Forward Declarations
class SidePostprocessor;

template<>
InputParameters validParams<SidePostprocessor>();

class SidePostprocessor :
  public SideUserObject,
  public Postprocessor
{
public:
  SidePostprocessor(const std::string & name, InputParameters parameters);

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where you want to do MPI communication!
   */
  virtual void finalize(){}
};

#endif
