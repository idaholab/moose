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

#include "Kernel.h"
#include "Postprocessor.h"

//Forward Declarations
class ElementPostprocessor;

template<>
InputParameters validParams<ElementPostprocessor>();

class ElementPostprocessor : public Kernel, public Postprocessor
{
public:
  ElementPostprocessor(const std::string & name, MooseSystem & moose_system, InputParameters parameters);  

  unsigned int blockID() { return _block_id; }
  
private:
  /**
   * The block ID this postprocessor works on
   */
  unsigned int _block_id;
  /**
   * Override the pure virtual... this function should NOT be overridden by other ElementPostprocessors
   */
  virtual Real computeQpResidual() { return 0; };
};
 
#endif
