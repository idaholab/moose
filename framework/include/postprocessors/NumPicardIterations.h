//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
