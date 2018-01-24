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

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward Declarations
class NumPicardIterations;
class Transient;

template <>
InputParameters validParams<NumPicardIterations>();

/**
 * Returns the number of Picard iterations taken by the underlying
 * Transient Executioner as a Postprocessor.
 */
class NumPicardIterations : public GeneralPostprocessor
{
public:
  NumPicardIterations(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  Transient * _transient_executioner;
};

#endif // NUMPICARDITERATIONS_H
