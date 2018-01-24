//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERALDAMPER_H
#define GENERALDAMPER_H

// Moose Includes
#include "Damper.h"

// Forward Declarations
class GeneralDamper;
class SubProblem;
class SystemBase;
class MooseVariable;
class Assembly;

template <>
InputParameters validParams<GeneralDamper>();

/**
 * Base class for deriving general dampers
 */
class GeneralDamper : public Damper
{
public:
  GeneralDamper(const InputParameters & parameters);

  /**
   * Computes this Damper's damping
   */
  virtual Real computeDamping(const NumericVector<Number> & solution,
                              const NumericVector<Number> & update) = 0;
};

#endif // GENERALDAMPER_H
