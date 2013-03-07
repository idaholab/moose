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

#ifndef PRINTVARS_H
#define PRINTVARS_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintNumVars;

template<>
InputParameters validParams<PrintNumVars>();

class PrintNumVars : public GeneralPostprocessor
{
public:
  PrintNumVars(const std::string & name, InputParameters parameters);

  virtual void initialize() {}

  virtual void execute() {}

  /**
   * This will return the number of elements in the system
   */
  virtual Real getValue();

protected:
  MooseEnum _system;
};

#endif //PRINTVARS_H
