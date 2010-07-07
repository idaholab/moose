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
