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

#ifndef NUMLINEARITERATIONS_H
#define NUMLINEARITERATIONS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumLinearIterations;

template <>
InputParameters validParams<NumLinearIterations>();

class NumLinearIterations : public GeneralPostprocessor
{
public:
  NumLinearIterations(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;
};

#endif // NUMLINEARITERATIONS_H
