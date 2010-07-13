#ifndef SIDEPOSTPROCESSOR_H
#define SIDEPOSTPROCESSOR_H

#include "BoundaryCondition.h"
#include "Postprocessor.h"

//Forward Declarations
class SidePostprocessor;

template<>
InputParameters validParams<SidePostprocessor>();

class SidePostprocessor : public BoundaryCondition, public Postprocessor
{
public:
  SidePostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters);  

private:
  /**
   * Override the pure virtual... this function should NOT be overridden by other SidePostprocessors
   */
  virtual Real computeQpResidual() { return 0; };
};
 
#endif
