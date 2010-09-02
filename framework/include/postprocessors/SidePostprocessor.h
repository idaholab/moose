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
