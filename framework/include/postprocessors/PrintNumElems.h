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

#ifndef PRINTELEMS_H
#define PRINTELEMS_H

#include "Postprocessor.h"

//Forward Declarations
class PrintNumElems;

template<>
InputParameters validParams<PrintNumElems>();

class PrintNumElems : public Postprocessor
{
public:
  PrintNumElems(std::string name, MooseSystem &moose_system, InputParameters parameters);
  
  virtual void initialize() {}
  
  virtual void execute() {}

  /**
   * This will return the number of elements in the system
   */
  virtual Real getValue();
};

#endif //PRINTELEMS_H
