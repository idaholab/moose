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

#ifndef ELEMENTPOSTPROCESSOR_H
#define ELEMENTPOSTPROCESSOR_H

#include "ElementUserObject.h"
#include "Postprocessor.h"

//Forward Declarations
class ElementPostprocessor;

template<>
InputParameters validParams<ElementPostprocessor>();

class ElementPostprocessor :
  public ElementUserObject,
  public Postprocessor
{
public:
  ElementPostprocessor(const std::string & name, InputParameters parameters);

  /**
   * Called before deleting the object. Free memory allocated by your derived classes, etc.
   */
  virtual void destroy(){}
};

#endif
