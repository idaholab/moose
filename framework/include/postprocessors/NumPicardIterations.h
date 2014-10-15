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

#ifndef NUMPICARDITERATIONS_H
#define NUMPICARDITERATIONS_H

#include "GeneralPostprocessor.h"
#include "Transient.h"

//Forward Declarations
class NumPicardIterations;

template<>
InputParameters validParams<NumPicardIterations>();

class NumPicardIterations : public GeneralPostprocessor
{
public:
  NumPicardIterations(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();
protected:
  Transient * _transient_executioner;
};

#endif // NUMPICARDITERATIONS_H
