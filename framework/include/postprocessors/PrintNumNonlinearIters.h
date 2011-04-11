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

#ifndef PRINTNUMNONLINEARITERS_H
#define PRINTNUMNONLINEARITERS_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintNumNonlinearIters;

template<>
InputParameters validParams<PrintNumNonlinearIters>();

class PrintNumNonlinearIters : public GeneralPostprocessor
{
public:
  PrintNumNonlinearIters(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();
};

#endif //PRINTNUMNONLINEARITERS_H
