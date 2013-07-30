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

#ifndef NUMNONLINEARITERATIONS_H
#define NUMNONLINEARITERATIONS_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class NumNonlinearIterations;

template<>
InputParameters validParams<NumNonlinearIterations>();

class NumNonlinearIterations : public GeneralPostprocessor
{
public:
  NumNonlinearIterations(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();
};

#endif // NUMNONLINEARITERATIONS_H
