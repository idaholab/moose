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

#ifndef ELEMENTPOSTPROCESSOR_H
#define ELEMENTPOSTPROCESSOR_H

#include "Kernel.h"
#include "Postprocessor.h"

//Forward Declarations
class ElementPostprocessor;

template<>
InputParameters validParams<ElementPostprocessor>();

class ElementPostprocessor : public Kernel, public Postprocessor
{
public:
  ElementPostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters);  

private:
  /**
   * Override the pure virtual... this function should NOT be overridden by other ElementPostprocessors
   */
  virtual Real computeQpResidual() { return 0; };
};
 
#endif
