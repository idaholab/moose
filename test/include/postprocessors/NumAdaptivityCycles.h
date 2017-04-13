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

#ifndef NUMADAPTIVITYCYCLES_H
#define NUMADAPTIVITYCYCLES_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumAdaptivityCycles;

template <>
InputParameters validParams<NumAdaptivityCycles>();

/**
 * Just returns the number of adaptivity cyles needed.
 */
class NumAdaptivityCycles : public GeneralPostprocessor
{
public:
  NumAdaptivityCycles(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;
};

#endif // NUMADAPTIVITYCYCLES_H
